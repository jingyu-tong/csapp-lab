# shelllab
这次实验目的是让我们熟悉进程控制以及信号。  
我们主要是要实现以下几个函数，使shell能够处理前后台运行程序，能够处理ctrl+c等信号：
* eval: 解析cmdline，并且运行
* builtin_cmd: 辨识和解析出buildin命令，quit，fg，bg和jobs
* waitfg: 等待前台程序运行结束
* sigchld_handler: 响应SIGCHLD(子进程终止)
* sigint_handler: 响应SIGINT(ctrl-c)信号
* sigtstp_handler: 响应SIGTSTP(ctrl-z)信号

实验提供了./tshref给我们参考输出结果，我们程序也就是仿照出这个的响应即可。

## 1.eval
eval解析用户命令并进行计算，对于前台进程，要执行到运行结束才能返回。有两点需要特别注意：
* 子进程默认和父进程属于同一个进程组，前台收到的关闭子进程信号，会同样被传递给统一进程组的父进程，导致一起退出，通过setgpid(0, 0)建立一个新的进程组可以避免
* ork进程前后要阻塞SIGCHLD，防止出现竞争，不阻塞，会出现子进程先结束从jobs中删除，在执行主进程addjob的竞争问题(csapp:3e p541)，相当于信号处理函数加锁被阻塞。我们在addjob之后开始处理SIGCHLD(类似解锁)，即可正确避免问题

## 2.builtin_cmd
这个函数比较简单，主要就是几个if配上strcmp就可以了，注意的是对于前台程序，我们要立即执行就好。

## 3.do_bgfg
主要是执行bg和fg的指令功能，在builtin_cmd中识别调用，也比较简单。
* 输出相关提示由trace文件配合./tshref获得
* 输入%num代表任务id，num代表进程id

## 4.waitfg
这个函数是用来等待前台进程完成的函数，因为SIGCHLD的信号处理函数也相当于这个功能，所以对于他们两个功能的区分，实验pdf给了如下两个方法：
* waifg利用sleep和循环，判断jobs是否还有前台状态的进程，信号处理函数负责回收僵尸进程
* 二者都回收僵尸进程

对于方法一，比较清晰，waitfg只负责等待，回收资源交给handler完成。方法二二者都调用了waitpid，虽然程序可能没问题，但是比较容易让人困惑，因此我们选用方案一。

## 5.信号处理函数
首先给我们提供了Signal函数进行信号handler的注册，里头封装了sigaction。程序主要处理如下几个信号
* SIGINT(ctrl + c)  
  这部分读取当前前台进程的pid，然后调用kill关闭前台进程组
* SIGTSTP(ctrl + z)  
  和ctrl+c类似，发送SIGTSTP到前台进程组
* SIGCHLD
  调用waitpid获取子进程信息，子进程如果退出，要删除jobs对应项，停止的话，要更新状态

## 总结
这次实验本身提供了框架和一些函数，首先要把提供的内容看看，做完这次实验，对信号和进程控制有着更深刻的认识。代码在[tsh.c](./shlab-handout/tsh.c)中。