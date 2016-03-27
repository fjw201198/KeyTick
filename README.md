# KeyTick
本程序模拟“机械键盘”发声。
这是一个 C 项目, 运行时需要root权限。 
本程序依赖于库openal、freeault  
本项目在Fedora 23 GNOME 下测试可用。


## 当前版本为1.0.0  


## 编译方法
进入项目的src目录，输入以下命令:  
1. `./configure`  
2. `make`  
3. `make install`  
### 注意，install的时候需要root权限  


## 安装后的清理工作  
执行 `make clean` 命令  


## 卸载方法  
执行 `make uninstall`, 需要root权限  

## 使用KeyTick而不安装的默认的路径  
将src下编译好的文件拷贝到bin下即可, 请保留sound目录，否则找不到声音文件。

# 安装KeyTick使用RPM
[KeyTick-1.0.0-1.x86_64.rpm](http://pan.baidu.com/s/1skLjwg1)  
点击上面的链接后，通过`rpm -ivh KeyTick-1.0.0-1.x86_64.rpm`即可完成安装。  
安装过程中，可能会遇到找不到alc.h, al.h, alut.h，这是因为你没有安装openal和freealut，通过以下命令来安装openal和freealut:  
`dnf install openal-soft freealut -y`

如果使用编译安装，还需要将devel包安装上:  
`dnf install openal-soft-devel freealut-devel -y`
