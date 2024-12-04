# Baidu-fanyi 

## 介绍
调用百度翻译api做的命令行翻译程序

自行安装所需要的库
curl
mbedtls
cjson

````
这两处可换成自己的
char *appid = ""; //appid
char *secret_key = "";//密钥
````

编译命令
`gcc -o main main.c -lcurl -lcjson -lmbedcrypto`



