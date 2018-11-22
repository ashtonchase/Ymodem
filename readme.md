#### About Source Code:

 - source 	: source code Of Ymodem Protocol Implemention

```c
source --
   |_____ common.h	: compile system header files needed to include
   |
   |_____ ymodem.c	: protocol implemention
   |_____ ymodem.h	: header file of protocol implemention
   |
   |_____ ymodem_util.c/.h 	: Tool Functions
   |
   |_____ ymodem_menu.c/.h 	: man - machine interaction menu

porting --
   |
   |_____ ymodem_export.h 	: All porting APIs should be Implemented by USER for different System or For different Usuage
   |_____ ymodem_export.c	: a demo implemention for porting APIs, For reference only, compile error exist 
   
```

 - porting	: Porting file. porting APIs declare in ymodem_export.h, and ymodem_export.c is for Xilinx Zynq

---


#### And Attention:

 - the origin source code is from Project\STM32F0xx_IAP Demo Project in ST.com

 - And you may read the blog-article to known more about Ymodem: [Ymodem Procotol Porting](https://nixlong.github.io/blog/2017/08/14/04-Mcu/[mcu]%20-%20Ymodem%E5%8D%8F%E8%AE%AE%E7%A7%BB%E6%A4%8D/)

---

By nix.long@126.com
By ashtonchase