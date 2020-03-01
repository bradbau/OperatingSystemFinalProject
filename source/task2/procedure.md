# 添加系统调用并编译内核
## 
以下内容是在fedora31上对linux kernel5.4.20进行编译的记录
实验基于成熟的开发工具rpmdevtools和包管理工具yum-utils等。
使用rpmbuild在编译的时候会需要一些额外的工具包，通过如下命令安装
$ sudo dnf install -y rpmdevtools numactl-devel pesign
$ sudo dnf groupinstall "Development Tools"
 sudo dnf build-dep kernel


2.21 su -c 'yum-builddep kernel-<version>.src.rpm'

## 编译内核

### 获得源码
1. 首先通过rpmdev-setuptree命令在用户主目录下准备建立 RPM 包的环境，这个命令新建了几个不同的目录 ${HOME}/rpmbuild/SOURCES ，${HOME}/rpmbuild/SPECS 和 ${HOME}/rpmbuild/BUILD 。${HOME} 是用户主目录。
2. 通过`yumdownloader --source kernel`下载源码文件，或者通过别的途径下载kernel-5.4.*.src.rpm 包

3. 在SOURCE目录下解压源码包
cd ~/rpmbuild/SOURCES
rpm2cpio kernel-* | cpio -i --make-directories
$ mv kernel-*.src.rpm ../SRPMS

4. 设置kernel.spec 也就是用来配置.config和makefile的指导文件
mv ~/rpmbuild/SOURCES/kernel.spec ~/rpmbuild/SPECS
$ cd ~/rpmbuild/SPECS
$ vi kernel.spec
在这一步的同时可以加补丁或者修改.spec的信息

5. 通过rombuild工具编译内核 
rpmbuild -ba ~/rpmbuild/SPECS/kernel.spec






- 问题1 在建立内核配置时，输入`rpmbuild -bb --target=`uname -m` kernel.spec`
出现编译信息后有以下内容
ernel/exit.o: warning: objtool: __x64_sys_exit_group()+0x14: unreachable instruction
kernel/cred.o: warning: objtool: get_task_cred.cold()+0x14: unreachable instruction
net/core/skbuff.o: warning: objtool: skb_push.cold()+0x15: unreachable instruction

libncurses++.so.6()(64bit) 被 ncurses-devel-6.1-12.20190803.fc31.x86_64 需要
	libncurses++w.so.6()(64bit) 被 ncurses-devel-6.1-12.20190803.fc31.x86_64 需要
	ncurses-c++-libs(x86-64) = 6.1-12.20190803.fc31 被 ncurses-devel-6.1-12.20190803.fc31.x86_64 需要


- 问题2 在编译内核时
  HOSTCC  scripts/basic/fixdep
*
* Unable to find the ncurses package.
* Install ncurses (ncurses-devel or libncurses-dev
* depending on your distribution).
*
需要安装依赖的库







- 问题 空间不足
解决方案时在线扩容
首先增加磁盘容量
fdisk /dev/sda中创建新的物理卷

pvcreate /dev/sda3
vgextend fedora_localhost-live /dev/sda3
lvextend -L +4.9G /dev/fedora_localhost-live/root
resize2fs /dev/mapper/fedora_localhost--live-root







