# Universal Error Code

Imagine universal error codes, just like unicode is for chars.

Comparing with characters each error tables today works a 
"code page" like windows error codes, or openssl error codes,
sqlite, linux etc.

A made some tables with the error name, value and message.

* winerror.h (errors from GetLastError)
* windows_errno.h (errors from windows errno)
* linux_errno.h (errors from linux errno)

How to build the universal error code table? Merging error code name or message?

In main.c there is a program that merges error codes by name.

Some error names like ENOTEMPTY are defined in windows and linux but
with diferent values.
