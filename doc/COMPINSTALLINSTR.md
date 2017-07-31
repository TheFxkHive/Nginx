# 编译安装指导  
##源码安装nginx   
	正式开始前，编译环境gcc g++开发库之类的需要提前装好，这里默认已经安装好了。 
## 编译并安装依赖库  
	CentOS平台编译环境可以使用如下指令：  
	安装make:  
	`yum -y install gcc automake autoconf libtool make`    

	安装g++：  
	`yum install gcc gcc-c++`  
	在下载安装源码之前，需要先安装nginx所依赖的库pcre、zlib、ssl  
#
	安装pcre   
	1.可以直接使用源码安装：  
	rewrite模块(pcre库):[http://www.pcre.org/](http://www.pcre.org/)  
	下载安装包：
	`wget http://downloads.sourceforge.net/project/pcre/pcre/8.21/pcre-8.21.tar.gz`  
	解压安装包:  
	`tar -zxvf pcre-8.21.tar.gz`  
	进入安装包目录：  
	`cd pcre-8.21`  
	编译安装：  
	`./configure`  
	`make && make install`  
	2.CentOS系统下可以直接使用命令安装：  
	查看是否已经安装：   
	`rpm -qa pcre`  
	如果没有安装，则使用命令：   
	`yum install -y pcre pcre-devel`  
#
	安装zlib
	1.可以直接使用源码安装：  
	gzlib模块(zlib库)：[http://www.pcre.org/](http://www.pcre.org/)  
	下载安装包：  
	`wget  http://zlib.net/zlib-1.2.8.tar.gz`  
	解压安装包：  
	`tar -zxvf zlib-1.2.8.tar.gz`  
	进入安装包目录：  
	`cd zlib-1.2.8`  
	编译安装：  
	`./configure`
	`make &&make install`  
	2.CentOS下可以直接使用命令安装：  
	查看是否已经安装：  
	`rpm -qa zlib`   
	如果没有安装，则使用命令：   
	`yum install -y zlib zlib-devel`
#
	安装ssl  
	1.可以直接使用源码安装：  
	openssl模块:[http://www.openssl.org/](http://www.openssl.org/)  
	下载安装包：  
	`wget https://www.openssl.org/source/openssl-1.0.1.tar.gz`  
	解压安装包：  
	`tar -zxvf openssl-1.0.1.tar.gz`  
	进入安装包目录：  
	`cd openssl-1.0.1`  
	编译安装：  
	`./configure`  
	`make && make install`  
	2.CentOS下可以使用命令安装：  
	`yum install openssl`
## 编译预处理 --configure选项  
语法：`./configure [OPTION]... [VAR=VALUE]...`
### 系统环境

### 配置路径
	--prefix=PATH		set installation prefix
	体系无关文件的顶级安装目录PREFIX
	--exec-prefix=EPREFIX
	体系相关文件的顶级安装目录EPREFIX，把体系相关的文件安装到不同的位置以方便不同主机之间共享体系相关的文件。[PREFIX]
	--bindir=DIR
	用户可执行目录DIR。二进制文件的安装位置。这里的二进制文件定义为可以被用户直接执行的程序
	--sbindir=DIR
	指定超级二进制文件的安装位置，这是一些通常只能由超级用户执行的程序
	--libexecdir=DIR
	指定可执行支持文件的安装位置，与二进制文件相反，这些文件从来不直接被用户执行，二而是被上面提到的二进制文件执行。
	--datadir=DIR
	指定数据文件的安装位置。
	--sysconfdir=DIR
	指定在单个机器上使用的只读数据的安装位置
	--sharedstatedir=DIR
	指定只能在多个机器上共享的可写数据的安装位置
	--localsttatedir=DIR
	指定只能在单机使用的可写数据的安装位置
	--libdir=DIR
	指定库文件的安装位置
	--includedir=DIR
	指定C头文件的安装位置，其他语言如C++的头文件也可以使用此选项
	--oldincudedir=DIR
	指定为除GCC外编译器安装的C头文件的安装位置
	--infodir=DIR
	指定Info格式文档的安装位置。
	--mandir=DIR
	指定手册页的安装位置
	--srcdir=DIR
	这个选项对安装没有作用，他会告诉'configure'源码的位置，一般来说不用指定此选项，'configure'脚本一般和源码在同一个目录下

### 增加模块

## 编译
--program-prefix=PREFIX  
指定将被加到所安装程序的名字上的前缀。  
--program-suffix=SUFFIX  
指定将被加到所安装程序的名字上的后缀。  
--program-transform-name=PROGRAM  
这类的PRODRAM是一个sed脚本，当一个程序被安装时，他的名字经过`sed -e PROGRAM`来产生安装的名字  
--build=BUILD  
指定软件运行的系统平台，如果没有指定，默认值将是'--host'选项的值。  
--host=HOST  
指定软件运行的系统平台，如果没有指定，将会运行`configure.guess`来检测  


# Nginx配置说明




