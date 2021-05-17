# yasio 中文文档

[![Docs Status](https://github.com/yasio/yasio/workflows/docs/badge.svg)](https://github.com/yasio/yasio/actions?query=workflow%3Adocs)

yasio 是一个轻量级跨平台的异步socket库，专注于客户端和基于各种游戏引擎的游戏客户端网络服务。

- 跨平台性:
    - 编译器: 
        - Visual Studio 2013+
        - GCC4.7+
        - xcode9+
        - 其他支持 C++11,14,17 的编译器

    - 架构: x86, x64, ARM等。
    - 操作系统: Windows, macOS, Linux, FreeBSD, iOS, Android等。

- 术语:
    - 网络服务: `io_service`
    - 信道: `io_channel`
    - 传输会话: `io_transport`

- 框架图:
![image](assets/images/framework.png)  

## 快速开始
此实例程序简单向 ``tool.chinaz.com`` 发送http请求并打印响应数据。

=== "C++"

    ```cpp
    #include "yasio/yasio.hpp"
    #include "yasio/obstream.hpp"
    using namespace yasio;
    using namespace yasio::inet;
    int main()
    {
        io_service service({"tool.chinaz.com", 80});
        service.set_option(YOPT_S_DEFERRED_EVENT, 0); // dispatch network event on network thread
        service.start([&](event_ptr&& ev) {
            switch (ev->kind())
            {
            case YEK_PACKET: {
                auto packet = std::move(ev->packet());
                fwrite(packet.data(), packet.size(), 1, stdout);
                fflush(stdout);
                break;
            }
            case YEK_CONNECT_RESPONSE:
                if (ev->status() == 0)
                {
                auto transport = ev->transport();
                if (ev->cindex() == 0)
                {
                    obstream obs;
                    obs.write_bytes("GET /index.htm HTTP/1.1\r\n");

                    obs.write_bytes("Host: tool.chinaz.com\r\n");

                    obs.write_bytes("User-Agent: Mozilla/5.0 (Windows NT 10.0; "
                                    "WOW64) AppleWebKit/537.36 (KHTML, like Gecko) "
                                    "Chrome/87.0.4820.88 Safari/537.36\r\n");
                    obs.write_bytes("Accept: */*;q=0.8\r\n");
                    obs.write_bytes("Connection: Close\r\n\r\n");

                    service.write(transport, std::move(obs.buffer()));
                }
                }
                break;
            case YEK_CONNECTION_LOST:
                printf("The connection is lost.\n");
                break;
            }
        });
        // open channel 0 as tcp client
        service.open(0, YCK_TCP_CLIENT);
        getchar();
    }
    ```

=== "Lua"

    ```lua
    local ip138 = "tool.chinaz.com"
    local service = yasio.io_service.new({host=ip138, port=80})
    local respdata = ""
    service:start(function(ev)
            local k = ev.kind()
            if (k == yasio.YEK_PACKET) then
                respdata = respdata .. ev:packet():to_string()
            elseif k == yasio.YEK_CONNECT_RESPONSE then
                if ev:status() == 0 then -- connect succeed
                    local transport = ev:transport()
                    local obs = yasio.obstream.new()
                    obs:write_bytes("GET / HTTP/1.1\r\n")

                    obs:write_bytes("Host: " .. ip138 .. "\r\n")

                    obs:write_bytes("User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36\r\n")
                    obs:write_bytes("Accept: */*;q=0.8\r\n")
                    obs:write_bytes("Connection: Close\r\n\r\n")

                    service:write(transport, obs)
                end
            elseif k == yasio.YEK_CONNECTION_LOST then
                print("request finish, respdata: " ..  respdata)
            end
        end)
    -- Open channel 0 as tcp client and start non-blocking tcp 3 times handshake
    service:open(0, yasio.YCK_TCP_CLIENT)

    -- Call this function at thread which focus on the network event.
    function gDispatchNetworkEvent(...)
        service:dispatch(128) -- dispatch max events is 128 per frame
    end

    _G.yservice = service -- Store service to global table as a singleton instance
    ```

## [测试](https://github.com/yasio/yasio/tree/master/tests) & [示例](https://github.com/yasio/yasio/tree/master/tests)

!!! attention "注意"

    运行Lua示例程序，会打印异常信息 `yasio - ibstream_view::consume out of range!`，这是示例程序里故意写的，请不必在意。

* 测试:
    * [echo_server](https://github.com/yasio/yasio/tree/master/tests/echo_server): TCP/UDP/KCP 回射服务器
    * [echo_client](https://github.com/yasio/yasio/tree/master/tests/echo_client): TCP/UDP/KCP 回射客户端
    * [ssltest](https://github.com/yasio/yasio/tree/master/tests/ssl): SSL测试客户端, 请求github.com主页并打印返回数据
    * [tcptest](https://github.com/yasio/yasio/tree/master/tests/tcp): TCP测试程序
    * [speedtest](https://github.com/yasio/yasio/tree/master/tests/speed): TCP,UDP,KCP 本机传输速率测试程序
    * [mcast](https://github.com/yasio/yasio/tree/master/tests/mcast): 组播测试程序

* 示例:
    * [ftp_server](https://github.com/yasio/ftp_server): 基于yasio实现的仅支持下载的ftp服务器，[点击](ftp://ftp.yasio.org/) 访问。
    * [lua](https://github.com/yasio/yasio/tree/master/examples/lua): Lua示例程序，包含简易的并发http请求，TCP拆包参数设置示例代码
    * [xlua](https://github.com/yasio/xLua): xlua集成案例
    * [DemoUE4](https://github.com/yasio/DemoUE4): UE4集成案例

## 编译 测试 & 示例
* 确保已安装支持C++11标准的编译器，例如 ``msvc``, ``gcc``, ``clang``
* 确保已安装 ``git``, ``cmake`` installed
* 运行如下命令:

```sh
  git clone https://github.com/yasio/yasio
  cd yasio
  git submodule update --init --recursive 
  cd build
  # for xcode should be: cmake .. -GXcode
  cmake ..
  cmake --build . --config Debug
```
