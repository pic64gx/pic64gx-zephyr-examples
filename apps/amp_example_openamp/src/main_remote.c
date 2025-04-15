/*
 * Copyright 2024 Microchip Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/drivers/ipm.h>

#include <openamp/open_amp.h>
#include <metal/sys.h>
#include <metal/io.h>
#include "resource_table.h"

#include <zephyr/logging/log.h>
#include <zephyr/console/console.h>
LOG_MODULE_REGISTER(openamp_rsc_table, LOG_LEVEL_DBG);

#define SHM_DEVICE_NAME	"shm"

#if !DT_HAS_CHOSEN(zephyr_ipc_shm)
#error "Sample requires definition of shared memory for rpmsg"
#endif

/* Constants derived from device tree */
#define SHM_NODE		DT_CHOSEN(zephyr_ipc_shm)
#define SHM_START_ADDR	DT_REG_ADDR(SHM_NODE)
#define SHM_SIZE		DT_REG_SIZE(SHM_NODE)

#define APP_TASK_STACK_SIZE (4096)

/* RPROC message to send to master to notify the remote is ready */
#define MCHP_IHC_RPROC_READY  (0xFFFFFF00)
/**
 * RPROC message to send by master to notify the remote must stop, only application cleanup is
 * left to user.
 */
#define MCHP_IHC_RPROC_STOP  (0xFFFFFF02)


#define PINGPONG_LOCAL_EPT_ADDR		(0)
#define PINGPONG_MAX_RPMSG_BUFF_SIZE 	(256)
/**
 * Why 40 I have no idea, there is 16 bytes of header and 24 bytes of padding
 * And the 24 bytes padding is a mistery to me
 */
#define PINGPONG_PAYLOAD_MAX_SIZE	(PINGPONG_MAX_RPMSG_BUFF_SIZE - 40)

K_THREAD_STACK_DEFINE(thread_mng_stack, APP_TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_app_stack, APP_TASK_STACK_SIZE);

static struct k_thread thread_mng_data;
static struct k_thread thread_app_data;

static const struct device *const ipm_handle =
	DEVICE_DT_GET(DT_CHOSEN(zephyr_ipc));

static metal_phys_addr_t shm_physmap = SHM_START_ADDR;
static metal_phys_addr_t rsc_tab_physmap;

static struct metal_io_region shm_io_data; /* shared memory */
static struct metal_io_region rsc_io_data; /* rsc_table memory */

struct rpmsg_rcv_msg {
	void *data;
	size_t len;
};

struct pingpong_payload {
    uint64_t num;
    uint64_t size;
    uint8_t data[PINGPONG_PAYLOAD_MAX_SIZE];
} __attribute__((__packed__));

static struct metal_io_region *shm_io = &shm_io_data;

static struct metal_io_region *rsc_io = &rsc_io_data;
static struct rpmsg_virtio_device rvdev;

static struct fw_resource_table *rsc_table;
static struct rpmsg_device *rpdev;

static char rx_sc_msg[20];  /* should receive "Hello world!" */
static struct rpmsg_endpoint sc_ept;
static struct rpmsg_rcv_msg sc_msg = {.data = rx_sc_msg};

static struct rpmsg_endpoint tty_ept;
static struct rpmsg_rcv_msg tty_msg;

static struct rpmsg_endpoint pingpomg_ept;
static struct rpmsg_rcv_msg pingpong_msg;

static K_SEM_DEFINE(data_sem, 0, 1);
static K_SEM_DEFINE(data_sc_sem, 0, 1);
static K_SEM_DEFINE(data_tty_sem, 0, 1);
static K_SEM_DEFINE(data_pingpong_sem, 0, 1);
static K_SEM_DEFINE(demo_start_sem, 0, 1);

static const uint8_t g_message[] =
"\r\n\r\n\r\n **** PIC64GX SoC Curiosity Kit AMP RPMsg Remote Zephyr Example ****\r\n\r\n\r\n";

static const uint8_t g_menu[] =
"\r\n\
Press 0 to show this menu\r\n\
Press 1 to run ping pong demo\r\n\
Press 2 to run console demo\r\n\
Press 3 to run sample echo demo\r\n";

static void cleanup_system(void);

void platform_rproc_ready()
{
	uint32_t ipm_message = MCHP_IHC_RPROC_READY;

	ipm_send(ipm_handle, 0, 0, &ipm_message, sizeof(ipm_message));

	k_sem_take(&data_sem,  K_FOREVER);
}

static void platform_ipm_callback(const struct device *dev, void *context,
				  uint32_t id, volatile void *data)
{
	uint32_t *msg = (uint32_t *)data;
	if(msg[0] == MCHP_IHC_RPROC_STOP) {
		cleanup_system();
	}
	k_sem_give(&data_sem);
}

static int rpmsg_recv_cs_callback(struct rpmsg_endpoint *ept, void *data,
				  size_t len, uint32_t src, void *priv)
{
	memcpy(sc_msg.data, data, len);
	sc_msg.len = len;
	k_sem_give(&data_sc_sem);

	return RPMSG_SUCCESS;
}

static int rpmsg_recv_tty_callback(struct rpmsg_endpoint *ept, void *data,
				   size_t len, uint32_t src, void *priv)
{
	struct rpmsg_rcv_msg *msg = priv;

	rpmsg_hold_rx_buffer(ept, data);
	msg->data = data;
	msg->len = len;
	k_sem_give(&data_tty_sem);

	return RPMSG_SUCCESS;
}

static int rpmsg_recv_pingpong_callback(struct rpmsg_endpoint *ept, void *data,
					size_t len, uint32_t src, void *priv)
{
	struct rpmsg_rcv_msg *msg = priv;
	rpmsg_hold_rx_buffer(ept, data);
	msg->data = data;
	msg->len = len;
	k_sem_give(&data_pingpong_sem);

	return RPMSG_SUCCESS;
}

static void receive_message(unsigned char **msg, unsigned int *len)
{
	int status = k_sem_take(&data_sem, K_FOREVER);

	if (status == 0) {
		rproc_virtio_notified(rvdev.vdev, VRING1_ID);
	}
}

static void new_service_cb(struct rpmsg_device *rdev, const char *name,
			   uint32_t src)
{
	LOG_ERR("%s: unexpected ns service receive for name %s",
		__func__, name);
}

int mailbox_notify(void *priv, uint32_t id)
{
	ARG_UNUSED(priv);
	int ret = 0;

	do {
		ret = ipm_send(ipm_handle, 1, id, &id, sizeof(id));
	} while (ret == -EBUSY);
	return 0;
}

int platform_init(void)
{
	int rsc_size;
	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
	int status;

	status = metal_init(&metal_params);
	if (status) {
		LOG_ERR("metal_init: failed: %d", status);
		return -1;
	}

	/* declare shared memory region */
	metal_io_init(shm_io, (void *)SHM_START_ADDR, &shm_physmap,
		      SHM_SIZE, -1, 0, NULL);

	/* declare resource table region */
	rsc_table_get((void **)&rsc_table, &rsc_size);

	rsc_tab_physmap = (uintptr_t)rsc_table;

	metal_io_init(rsc_io, rsc_table,
		      &rsc_tab_physmap, rsc_size, -1, 0, NULL);

	/* setup IPM */
	if (!device_is_ready(ipm_handle)) {
		LOG_ERR("IPM device is not ready");
		return -1;
	}

	ipm_register_callback(ipm_handle, platform_ipm_callback, NULL);

	status = ipm_set_enabled(ipm_handle, 1);
	if (status) {
		LOG_ERR("ipm_set_enabled failed");
		return -1;
	}

	return 0;
}

static void cleanup_system(void)
{
	ipm_set_enabled(ipm_handle, 0);
	rpmsg_deinit_vdev(&rvdev);
	metal_finish();
}

struct  rpmsg_device *
platform_create_rpmsg_vdev(unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_cb)
{
	struct fw_rsc_vdev_vring *vring_rsc;
	struct virtio_device *vdev;
	int ret;
	uintptr_t virt_addr;

	vdev = rproc_virtio_create_vdev(VIRTIO_DEV_DEVICE, VDEV_ID,
					rsc_table_to_vdev(rsc_table),
					rsc_io, NULL, mailbox_notify, NULL);

	if (!vdev) {
		LOG_ERR("failed to create vdev");
		return NULL;
	}

	platform_rproc_ready();

	/* wait master rpmsg init completion */
	rproc_virtio_wait_remote_ready(vdev);

	vring_rsc = rsc_table_get_vring0(rsc_table);
	virt_addr = vring_rsc->da;
	ret = rproc_virtio_init_vring(vdev, 0, vring_rsc->notifyid,
				      (void *)virt_addr, rsc_io,
				      vring_rsc->num, vring_rsc->align);
	if (ret) {
		LOG_ERR("failed to init vring 0");
		goto failed;
	}

	vring_rsc = rsc_table_get_vring1(rsc_table);
	virt_addr = vring_rsc->da;
	ret = rproc_virtio_init_vring(vdev, 1, vring_rsc->notifyid,
				      (void *)virt_addr, rsc_io,
				      vring_rsc->num, vring_rsc->align);
	if (ret) {
		LOG_ERR("failed to init vring 1");
		goto failed;
	}

	ret = rpmsg_init_vdev(&rvdev, vdev, ns_cb, shm_io, NULL);
	if (ret) {
		LOG_ERR("failed rpmsg_init_vdev");
		goto failed;
	}

	return rpmsg_virtio_get_rpmsg_device(&rvdev);

failed:
	rproc_virtio_remove_vdev(vdev);

	return NULL;
}

/* This is the main function for the ping pong application demo.
 *
 * When in RPMsg master mode, it sends payloads or variable size to the RPMsg
 * remote context and validates the data echoed back by the software context
 * configured as RPMsg remote.
 *
 * When in RPMsg remote mode, this function received and echoes back payloads
 * sent by the RPMsg master side.
 */
 void rpmsg_pingpong_demo(void)
{
	int ret = 0;
	struct pingpong_payload * r_payload;
	unsigned char tx_buff[PINGPONG_MAX_RPMSG_BUFF_SIZE];
	uint32_t i;

	printf( "\r\nSending name service announcement\r\n");
	printf( "\r\nPlease run pingpong demo on the RPMsg master context\r\n\r\n");
	pingpomg_ept.priv = &pingpong_msg;
	ret = rpmsg_create_ept(&pingpomg_ept, rpdev, "rpmsg-amp-demo-channel",
			       PINGPONG_LOCAL_EPT_ADDR, RPMSG_ADDR_ANY,
			       rpmsg_recv_pingpong_callback, NULL);
	if (ret) {
		LOG_ERR("[Linux sample client] Could not create endpoint: %d", ret);
		goto task_end;
	}

	for (i = 0; i < PINGPONG_PAYLOAD_MAX_SIZE; i++)
	{
		k_sem_take(&data_pingpong_sem,  K_FOREVER);

		r_payload = (struct pingpong_payload*) pingpong_msg.data;

		printf( "received payload number ");
		printf( "%llu of size %ld\r\n", r_payload->num, pingpong_msg.len);
		memcpy(tx_buff, pingpong_msg.data, pingpong_msg.len);
		rpmsg_release_rx_buffer(&pingpomg_ept, pingpong_msg.data);
		rpmsg_send(&pingpomg_ept, tx_buff, pingpong_msg.len);
	}

	printf( "\r\nEnd of ping pong demo. Press 0 to show menu\r\n");
	rpmsg_destroy_ept(&pingpomg_ept);

task_end:
	LOG_INF("OpenAMP Linux pingpong client responder ended");
}

/* This is the main function for the sample echo application demo.
 *
 * When in RPMsg remote mode, this application sends a configurable number of
 * hello messages to the software context configured as RPMsg master
 *
 * When in RPMsg master mode, this application received and echoes back messages
 * received by the software context configured as RPMsg master.
 */
void rpmsg_echo_demo(void)
{
	unsigned int msg_cnt = 0;
	unsigned int err_cnt = 0;
	int ret = 0;

	printf( "\r\nSending name service announcement\r\n");
	printf( "\r\nPlease run sample echo demo on the RPMsg master context\r\n\r\n");
	ret = rpmsg_create_ept(&sc_ept, rpdev, "rpmsg-client-sample",
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_recv_cs_callback, NULL);
	if (ret) {
		LOG_ERR("[Linux sample client] Could not create endpoint: %d", ret);
		goto task_end;
	}

	while (msg_cnt < 100) {
		k_sem_take(&data_sc_sem,  K_FOREVER);
		msg_cnt++;
		printf("received message %d: ", msg_cnt);
		fwrite(sc_msg.data, 1, sc_msg.len, stdout);
		printf(" of size %lu \r\n", (unsigned long) sc_msg.len);
		if (strncmp(sc_msg.data, "hello world!", sc_msg.len)) {
			printf( " Invalid message is received.\r\n");
			err_cnt++;
		}
		printf( "rpmsg sample test: message %d sent\r\n", msg_cnt);
		rpmsg_send(&sc_ept, sc_msg.data, sc_msg.len);
	}

	printf( "**********************************\r\n");
	printf( " Test Results: Error count = %d\r\n", err_cnt);
	printf( "**********************************\r\n");
	printf( "\r\nEnd of sample echo demo. Press 0 to show menu\r\n");
	rpmsg_destroy_ept(&sc_ept);

task_end:
	LOG_INF("OpenAMP Linux sample client responder ended");
}

/* This is the main function for the console application demo.
 *
 * The software context configured as RPMsg master receives strings from the
 * UART1 console and sends them to the software context configured as RPMsg
 * remote using the RPMsg framework.
 *
 * The software context configured as RPMsg remote receives the messages
 * and prints them on the UART2s console.
 */
 void rpmsg_console_demo(void)
 {
	bool quit_message = false;
	int ret = 0;

	printf("\r\nSending name service announcement\r\n");
	printf("\r\nPlease run console demo on the RPMsg master context\r\n\r\n");

	tty_ept.priv = &tty_msg;
	ret = rpmsg_create_ept(&tty_ept, rpdev, "rpmsg-tty",
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_recv_tty_callback, NULL);
	if (ret) {
		LOG_ERR("[Linux TTY] Could not create endpoint: %d", ret);
		goto task_end;
	}

	while ((tty_ept.addr !=  RPMSG_ADDR_ANY) && !quit_message) {
		k_sem_take(&data_tty_sem,  K_FOREVER);
		if (tty_msg.len) {
			fwrite(tty_msg.data, 1, tty_msg.len, stdout);
			printf("\r\n");

			if(!strncmp(tty_msg.data, "quit", tty_msg.len))
			{
				quit_message = true;
				rpmsg_release_rx_buffer(&tty_ept, tty_msg.data);
				tty_msg.len = 0;
				tty_msg.data = NULL;
				printf("\r\nEnd of console/tty demo. Press 0 to show menu\r\n");
			}
		}
		rpmsg_release_rx_buffer(&tty_ept, tty_msg.data);
		tty_msg.len = 0;
		tty_msg.data = NULL;
	}
	rpmsg_destroy_ept(&tty_ept);

task_end:
	LOG_INF("OpenAMP Linux TTY responder ended");
 }

void app_rpmsg(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	uint8_t rx_buff[1];
	uint8_t rx_size = 0;

	console_init();

	k_sem_take(&demo_start_sem,  K_FOREVER);

	printf("%s", g_message);

	printf("%s", g_menu);

	while(1)
	{
	    rx_size = console_read(NULL, rx_buff, sizeof(rx_buff));
	    if (rx_size > 0)
	    {
		switch(rx_buff[0])
		{
		    case '0':
			printf("%s", g_menu);
			break;
		    case '1':
			rpmsg_pingpong_demo();
			break;
		    case '2':
			rpmsg_console_demo();
			break;
		    case '3':
			rpmsg_echo_demo();
			break;
		    default:
			break;
		}
	    }
	}
}

void rpmsg_mng_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	unsigned char *msg;
	unsigned int len;
	int ret = 0;

	LOG_INF("OpenAMP[remote] Linux responder demo started");

	/* Initialize platform */
	ret = platform_init();
	if (ret) {
		LOG_ERR("Failed to initialize platform");
		ret = -1;
		goto task_end;
	}

	rpdev = platform_create_rpmsg_vdev(0, VIRTIO_DEV_DEVICE, NULL,
					   new_service_cb);
	if (!rpdev) {
		LOG_ERR("Failed to create rpmsg virtio device");
		ret = -1;
		goto task_end;
	}

	/* start the app client */
	k_sem_give(&demo_start_sem);

	while (1) {
		receive_message(&msg, &len);
	}

task_end:
	cleanup_system();

	LOG_INF("OpenAMP demo ended");
}

int main(void)
{
	LOG_INF("Starting application threads!");
	k_thread_create(&thread_mng_data, thread_mng_stack, APP_TASK_STACK_SIZE,
			rpmsg_mng_task,
			NULL, NULL, NULL, K_PRIO_COOP(8), 0, K_NO_WAIT);

	k_thread_create(&thread_app_data, thread_app_stack, APP_TASK_STACK_SIZE,
			app_rpmsg,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);
	return 0;
}
