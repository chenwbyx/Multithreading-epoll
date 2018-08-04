Multithreading-epoll，一个基于epoll的多线程服务器
===
>Design and maintenance by chenwenbo.
* 基于epoll，监听指定端口的TCP请求；
* TCP长连接、短连接分别使用不同的端口；
* 当接收到客户端的TCP短连接请求时，返回服务器ip地址、MAC地址；当接收到客户端的TCP长连接请求时，服务器接收连接，客户端周期性向服务器发送心跳；
* 支持多个客户端的并发TCP请求；
* 使用多线程技术，同时监听、提供服务；
* 设计服务器端和客户端的通信协议，使用JSON格式。
