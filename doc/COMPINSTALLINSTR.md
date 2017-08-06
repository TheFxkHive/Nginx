# 编译安装指导  
## 源码安装nginx   
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

### 编译
	--program-prefix=PREFIX  
	指定将被加到所安装程序的名字上的前缀。  
	--program-suffix=SUFFIX  
	指定将被加到所安装程序的名字上的后缀。  
	--program-transform-name=PROGRAM  
	这类的PRODRAM是一个sed脚本，当一个程序被安装时，他的名字经过`sed -e PROGRAM`来产生安装的名字 
	--build=BUILD 
	指定软件运行的系统平台，如果没有指定，默认值将是'--host'选项的值。  
	--host=HOST  
	指定软件运行的系统平台，如果没有指定，将会运行`configure.guess`来检测。    
	--target=GARGET
	指定软件面向(target to)的系统平台。这主要在程序语言工具如编译器和汇编器上下文中起作用。如果没有指定，默认将使用`--host`选项的值。  
	--disable-FEATURE  
	一些软件包可以选择这个选项来提供为大型选项的编译时配置。例如使用Kerberos认证系统或者一个实验性编译器最优配置，如果默认时提供这些特性，可以使		用这些特性，可以使用'--disable-FEATURE'来禁用它，这里的'FEATURE'是特性的名字。  
	--enable-FEATURE[=ARG]  
	相反的，一些软件包可能提供了一些默认被禁止的特性，可以使用'--enable-FEATURE'来启用它，这里的'FEATURE'是特性的名字，一个特性可能会接受一个		可选的参数。  
	--without-PACKAGE  
	有时候你可能不想让你的软件包与系统已有软件包交互。例如，你可能不想让你的新编译器使用GUN ld。通过使用这个选项可以做到这一点：  
	$./comfigure --without-gnu-ld  
	--x-includes=DIR  
	这个选项是'--with-PACKAGE'选项的一个特例，在Autoconf最初被开发出来时，流行使用'configure'来作为lmake的一个变通方法来制作运行于X的软件'--		x-includes'选项提供了向'configure'脚本指明包含X11头文件的目录的方法。  
	--x-libraries=DIR  
	类似的，'--x-libraries'选项提供了向'configure'脚本指明包含X11库的目录的方法。  


# Nginx配置说明  
	#user  nobody;  
	#指定nginx运行的用户及用户组，默认为nobody 
	worker_processes  1;  
	#nginx进程数，建议设置为等于CPU总核心数。
	
	#error_log  logs/error.log;
	#error_log  logs/error.log  notice;
	#error_log  logs/error.log  info;
	#定于全局错误日志文件，级别以notice显示。还有debug、info、warn、error、crit模式，debug输出最多，crit输出最少，更加实际环境而定。

	#pid        logs/nginx.pid;
	#指定进程id的存储文件位置
	
	worker_rlimit_nofile 655355;  
	#指定一个nginx进程打开的最多文件描述符数目，受系统进程的最大打开文件描述符的数量限制
																		events {
		use epoll;
		#参考事件模型，use [kqeue | rtsing | epoll |/dev/poll| select | poll],epoll模型是Linux2.6以上版本内核中的高性能网络I/O模型，			如果跑在FreeBSD上，就用kqueue;
		worker_connections  1024;  
		#定义每个进程的最大连接数，受系统进程的最大打开文件描述符数量的限制,(最大连接数=连接*进程数)
	
	}

	//设定http服务器
	http {
		include       mime.types;  
		#文件扩展名与文件型映射表
		
		default_type  application/octet-stream;
		#默认文件类型
		
		#log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
		#'$status $body_bytes_sent "$http_referer" '
		#'"$http_user_agent" "$http_x_forwarded_for"';

		#access_log  logs/access.log  main;

		sendfile        on;
		#tcp_nopush     on;

		#keepalive_timeout  0;
		keepalive_timeout  65;

		#gzip  on;

		server {
		 listen       80;
		server_name  localhost;
		#charset koi8-r;
		#access_log  logs/host.access.log  main;
		
		location / {
			 root   html;
			 index  index.html index.htm;
			#error_page  404              /404.html;
			# redirect server error pages to the static page /50x.html
																				#error_page   500 502 503 504  /50x.html;
			location = /50x.html {
				root   html;
			}
			
			# proxy the PHP scripts to Apache listening on 127.0.0.1:80
																				#
			 #location ~ \.php$ {
			 	#proxy_pass   http://127.0.0.1;														        
				}
			# pass the PHP scripts to FastCGI server listening on 127.0.0.1:9000
																				#
			#location ~ \.php$ {
				 # root	html;
				 # fastcgi_pass   127.0.0.1:9000;
				# fastcgi_index  index.php;
				# fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
				# include        fastcgi_params;
				#
			#}
			
			# deny access to .htaccess files, if Apache's document root
			# concurs with nginx's one
			#
			#location ~ /\.ht {
			#    deny  all;
			#}
																	 	}

	 # another virtual host using mix of IP-, name-, and port-based configuration
	 #
	 #server {
	 	#    listen       8000;
		#    listen       somename:8080;
		#    server_name  somename  alias  another.alias;
																
		#    location / {
																				#        root   html;
																				#        index  index.html index.htm;
																				#    }
																		#}
																		# HTTPS server
																		#
																			#server {
																				#    listen       443 ssl;
			#    server_name  localhost;
			#    ssl_certificate      cert.pem;
			#    ssl_certificate_key  cert.key;
			#    ssl_session_cache    shared:SSL:1m;
			#    ssl_session_timeout  5m;
			#    ssl_ciphers  HIGH:!aNULL:!MD5;
			#    ssl_prefer_server_ciphers  on;
																				#    location / {
																					#        root   html;
																					#        index  index.html index.htm;
																				#    }
																			#}
	}



