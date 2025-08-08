# bin文件查看

bin文件是二进制文件，需要使用十六进制编辑器查看，如HxD。

bin文件的格式如下：

0x00000000: 7280 0000 0500 0000 7300 0000 0600 0000 

其中，例如：

```bash
hexdump -C examples/test_first_version.bin | head -10
```