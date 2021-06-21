---
title: "yasio::inet::io_service Class"
date: "12/11/2020"
f1_keywords: ["io_service", "yasio/io_service", ]
helpviewer_keywords: []
---
# io_service Class

充分利用 `socket.select` 多路复用模型实现网络服务，提供给上层统一的接口来进行 `tcp, udp, kcp, ssl-client` 通信。

## 语法

```cpp
namespace yasio { inline namespace inet { class io_service; } }
```

## 成员

### 公共构造函数

|Name|Description|
|----------|-----------------|
|[io_service::io_service](#io_service)|构造1个 `io_service` 对象|

### 公共方法

|Name|Description|
|----------|-----------------|
|[io_service::start](#start)|启动网络服务线程|
|[io_service::stop](#stop)|停止网络服务线程|
|[io_service::open](#open)|打开信道|
|[io_service::close](#close)|关闭传输会话|
|[io_service::is_open](#is_open)|检测信道或会话是否打开|
|[io_service::dispatch](#dispatch)|分派网络事件|
|[io_service::write](#write)|异步发送数据|
|[io_service::write_to](#write_to)|异步发送DGRAM数据|
|[io_service::schedule](#schedule)|注册定时器|
|[io_service::init_globals](#init_globals)|显示初始化全局数据|
|[io_service::cleanup_globals](#cleanup_globals)|清理全局数据|
|[io_service::channel_at](#channel_at)|获取信道句柄|
|[io_service::set_option](#set_option)|设置选项|

## 注意

默认传输会话的创建会使用对象池 `object_pool`。

## 要求

**头文件:** yasio.hpp

## <a name="io_service"></a> io_service::io_service

构造 `io_service` 对象。

```cpp
io_service::io_service();

io_service::io_service(int channel_count);

io_service::io_service(const io_hostent& channel_ep);

io_service::io_service(const io_hostent* channel_eps, int channel_count);
```

### 参数

*channel_count*<br/>
信道数量。

*channel_ep*<br/>
信道远端地址。

*channel_eps*<br/>
信道远端地址数组首地址。

### 示例

```cpp
#include "yasio/yasio.hpp"
int main() {
    using namespace yasio;
    io_service s1; // s1 only support 1 channel
    io_service s2(5); // s2 support 5 channels concurrency
    io_service s3(io_hostent{"github.com", 443}); // s3 support 1 channel
    io_hostent hosts[] = {  
        {"192.168.1.66", 20336},
        {"192.168.1.88", 20337},
    };
    io_service s4(hosts, YASIO_ARRAYSIZE(hosts)); // s4 support 2 channels concurrency
    return 0;
}
```

## <a name="start"></a> io_service::start

启动网络服务线程。

```cpp
void start(io_event_cb_t cb);
```

### 参数

*cb*<br/>
网络事件回调，默认情况下在 [io_service::dispatch](#dispatch) 调用者线程调度。

### 示例

```cpp
#include "yasio/yasio.hpp"
int main() {
    using namespace yasio;
    auto service = yasio_shared_service(io_hostent{host="ip138.com", port=80});
    service->start([](event_ptr&& ev) {
    auto kind = ev->kind();
    if (kind == YEK_ON_OPEN)
    {
        if (ev->status() == 0)
        printf("[%d] connect succeed.\n", ev->cindex());
        else
        printf("[%d] connect failed!\n", ev->cindex());
    }
    });
    return 0;
}
```

## <a name="stop"></a> io_service::stop

停止网络服务线程。

```cpp
void stop()
```

### 注意

如果网络服务线程正在运行, 此函数会发送退出信号并等待其正常退出。

## <a name="open"></a> io_service::open

打开信道。

```cpp
void open(size_t cindex, int kind);
```

### 参数

*cindex*<br/>
信道索引。

*kind*<br/>
信道类型。

### 注意

对于`TCP`, 将会请求内核发起非阻塞3次握手来建立可靠连接。

*cindex* 的值必须小于io_service初始化时的信道数量。

*kind* 必须是以下枚举值之一:

- `YCK_TCP_CLIENT`
- `YCK_TCP_SERVER`
- `YCK_UDP_CLIENT`
- `YCK_UDP_SERVER`
- `YCK_KCP_CLIENT`
- `YCK_KCP_SERVER`
- `YCK_SSL_CLIENT`


## <a name="close"></a> io_service::close

关闭信道或者传输会话。

```cpp
void close(transport_handle_t transport);

void close(int cindex);
```

### 参数

*transport*<br/>
将要关闭的传输会话。

*cindex*<br/>
将要关闭的信道。

### 注意

对于`TCP`， 将会发起4次握手来终止连接。

## <a name="is_open"></a> io_service::is_open

判断信道或传输会话是否处于打开状态。

```cpp
bool is_open(transport_handle_t transport) const;

bool is_open(int cindex) const;
```

### 参数

*transport*<br/>
传输会话句柄。

*cindex*<br/>
信道索引。


### 返回值
`true`: 打开，`false`: 未打开

## <a name="dispatch"></a> io_service::dispatch

分派网络线程产生的事件。

```cpp
void dispatch(int max_count);
```

### 参数

*max_count*<br/>
本次最大分派事件数。

### 注意

通常此方法应当在关心网络事件的业务逻辑线程调用, 例如Cocos2d-x的渲染线程，以及其他游戏引擎(Unity,UE4)的主逻辑线程。

此方法对于安全地更新游戏界面非常有用。

### 示例

```cpp
yasio_shared_service()->dispatch(128);
```

## <a name="write"></a> io_service::write

向传输会话远端发送数据。

```cpp
int write(
    transport_handle_t thandle,
    std::vector<char> buffer,
    io_completion_cb_t completion_handler = nullptr
);
```

### 参数

*thandle*<br/>
传输会话句柄。

*buffer*<br/>
要发送的二进制缓冲区。

*completion_handler*<br/>
发送完成回调。

### 返回值

返回发送数据字节数, `< 0`: 说明发生错误。

### 注意

*completion_handler* 不支持 `KCP`。

空buffer会直接被忽略，也不会触发 *completion_handler* 。

## <a name="write_to"></a> io_service::write_to

向UDP传输会话发送数据。

```cpp
int write_to(
    transport_handle_t thandle,
    std::vector<char> buffer,
    const ip::endpoint& to,
    io_completion_cb_t completion_handler = nullptr
);
```

### 参数

*thandle*<br/>
传输会话句柄。

*buffer*<br/>
要发送的buffer。

*to*<br/>
要发送的远端地址。

*completion_handler*<br/>
发送完成回调。

### 返回值

成功发送的字节数, 当 `< 0` 时说明发生错误，通常是传输会话已关闭(TCP连接断开等)。

### 注意

此函数仅可用于 *DGRAM* 传输会话，即 `UDP,KCP`。

发送完成回调 *completion_handler* 不支持 `KCP`。

空buffer会直接被忽略，也不会触发 *completion_handler* 。

## <a name="schedule"></a> io_service::schedule

注册一个定时器。

```cpp
highp_timer_ptr schedule(
    const std::chrono::microseconds& duration,
    timer_cb_t cb
);
```

### 参数

*duration*<br/>
定时器超时时间，毫秒级。

*cb*<br/>
当定时器到期后回调。

### 返回值

`std::shared_ptr` 包装的定时器对象，以便用户对定时器安全地进行必要操作。

### 示例

```cpp
// Register a once timer, timeout is 3 seconds.
yasio_shared_service()->schedule(std::chrono::seconds(3), []()->bool{
  printf("time called!\n");
  return true;
});

// Register a loop timer, interval is 5 seconds.
auto loopTimer = yasio_shared_service()->schedule(std::chrono::seconds(5), []()->bool{
  printf("time called!\n");
  return false;
});
```

## <a name="init_globals"></a> io_service::init_globals

静态方法，显示地初始化全局数据

```cpp
static void init_globals(print_fn2_t print_fn);
```

### 参数

*print_fn*<br/>
自定义网络日志打印函数。

### 注意

此函数是可选调用，但是当用户需要将网络日志重定向到自定义日志系统时，<br/>
则非常有用，例如重定向到UE4和U3D的日志输出。

### 示例

```cpp
// yasio_uelua.cpp
// compile with: /EHsc
#include "yasio_uelua.h"
#include "yasio/platform/yasio_ue4.hpp"
#include "lua.hpp"
#if defined(NS_SLUA)
using namespace NS_SLUA;
#endif
#include "yasio/bindings/lyasio.cpp"

DECLARE_LOG_CATEGORY_EXTERN(yasio_ue4, Log, All);
DEFINE_LOG_CATEGORY(yasio_ue4);

void yasio_uelua_init(void* L)
{
  auto Ls            = (lua_State*)L;
  print_fn2_t log_cb = [](int level, const char* msg) {
    FString text(msg);
    const TCHAR* tstr = *text;
    UE_LOG(yasio_ue4, Log, L"%s", tstr);
  };
  io_service::init_globals(log_cb);

  luaregister_yasio(Ls);
}
void yasio_uelua_cleanup()
{
  io_service::cleanup_globals();
}
```

## <a name="cleanup_globals"></a> io_service::cleanup_globals

静态方法，显示地清理全局数据。

```cpp
static void cleanup_globals();
```

### 注意

当用户需要卸载包含自定义日志打印回调的动态库(.dll,.so)前必须调用此函数，谨防应用程序闪退。

## <a name="channel_at"></a> io_service::channel_at

通过信道索引获取信道句柄。

```cpp
io_channel* channel_at(size_t cindex) const;
```

### 参数

*cindex*<br/>
信道索引

### 返回值

信道句柄指针, 当索引值超出范围时，返回 `nullptr`。

## <a name="set_option"></a> io_service::set_option

设置选项。

```cpp
void set_option(int opt, ...);
```

### 参数

*opt*<br/>
选项枚举, 请查看 [YOPT_X_XXX](io_service-options.md).

### 示例

```cpp
#include "yasio/yasio.hpp"

int main(){
    using namespace yasio;
    io_hostent hosts[] = {
    {"192.168.1.66", 20336},
    {"192.168.1.88", 20337},
    };
    auto service = std::make_shared<io_service>(hosts, YASIO_ARRAYSIZE(hosts));

    // for application protocol with length field, you just needs set this option.
    // it's similar to java netty length frame based decode.
    // such as when your protocol define as following
    //    packet.header: (header.len=12bytes)
    //           code:int16_t
    //           datalen:int32_t (not contains packet.header.len)
    //           timestamp:int32_t
    //           crc16:int16_t
    //    packet.data
    service->set_option(YOPT_C_LFBFD_PARAMS,
                        0,     // channelIndex, the channel index
                        65535, // maxFrameLength, max packet size
                        2,     // lenghtFieldOffset, the offset of length field
                        4,     // lengthFieldLength, the size of length field, can be 1,2,4
                        12    // lengthAdjustment：if the value of length feild == packet.header.len + packet.data.len, this parameter should be 0, otherwise should be sizeof(packet.header)
    );

    // for application protocol without length field, just sets length field size to -1.
    // then io_service will dispatch any packet received from server immediately,
    // such as http request, this is default behavior of channel.
    service->set_option(YOPT_C_LFBFD_PARAMS, 1, 65535, -1, 0, 0);
    return 0;
}
```

## 请参阅

[io_event Class](./io_event-class.md)

[io_channel Class](./io_channel-class.md)

[io_service Options](./io_service-options.md)

[xxsocket Class](./xxsocket-class.md)

[obstream Class](./obstream-class.md)

[ibstream_view Class](./ibstream-class.md)

[ibstream Class](./ibstream-class.md#ibstream-class)
