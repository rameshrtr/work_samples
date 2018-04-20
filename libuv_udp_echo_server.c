#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"

#define CHECK(r, msg)                                       \
	if (r<0) {                                              \
		fprintf(stderr, "%s: %s\n", msg, uv_strerror(r));   \
		exit(1);                                            \
	}

static uv_loop_t *uv_loop;
static uv_udp_t   recv_socket;

void on_send(uv_udp_send_t *req, int status) {
	if (status) {
		fprintf(stderr, "Send error %s\n", uv_strerror(status));
		return;
	}
	free(req);
}

static void on_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags) {
	int status = -1;
	if(addr == NULL || nread <= 0){
		return;
	}

	uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
	
	char sender[17] = "";
	uv_buf_t sndbuf = uv_buf_init(rcvbuf->base, nread);
	uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);

	fprintf(stderr, "Recv from client: [%s], Len: [%d], Data ptr: [%p]\n", sender, nread, rcvbuf->base);

	if(status = uv_udp_send(send_req, handle, (const struct uv_buf_t *)&sndbuf, 1, (const struct sockaddr *)addr, on_send))
	{
		fprintf(stderr, "Send error on_rcv %s\n", uv_strerror(status));
	}
	free(rcvbuf->base);
}

static void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
	buf->base = malloc(suggested_size);
	buf->len = suggested_size;
	memset(buf->base, 0x00, suggested_size);
	/*printf("malloc:%lu %p\n",buf->len,buf->base);*/
}

int main(int argc,char *argv[]) {
	int status;
	struct sockaddr_in addr;
	struct sockaddr_in saddr;
	
	fprintf(stderr, "libuv version:%s\n", uv_version_string());
	uv_loop = uv_default_loop();
	//recv socket
	status = uv_udp_init(uv_loop,&recv_socket);
	CHECK(status,"init");
	uv_ip4_addr("0.0.0.0", 11000, &addr);
	status = uv_udp_bind(&recv_socket, (const struct sockaddr*)&addr,UV_UDP_REUSEADDR);
	CHECK(status,"bind");
	status = uv_udp_recv_start(&recv_socket, on_alloc, on_recv);
	CHECK(status,"recv");
	uv_run(uv_loop, UV_RUN_DEFAULT);

	return 0;
}
