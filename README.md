# NachOSJorge

This repository consists of the NachOS project, as described on the file nachos.ps, plus some additional modifications implemented by me. Since NachOS is a tool to help computer science students learn how operating systems work by playing around with a virtual machine, my assignment was to write the code necessary to perform system calls to provide synchronization, file usage and virtual memory to the NachOS virtual machine. Most of the code in this project is readily available from the University of Berkeley, and I do not claim to have written it.

In this project, I modified the following files to implement said processes:  
`/code/userprog/addrspace.cc`  
`/code/userprog/nachostabla.cc`  
`/code/userprog/nachostabla.h`  
`/code/userprog/nachosthreads.cc`  
`/code/userprog/nachosthreads.h`  
`/code/userprog/semaforoTabla.cc`  
`/code/userprog/semafotoTabla.h`  
`/code/userprog/exception.cc`  
`/code/machine/translate.cc`  
`/code/machine/machine.cc`

Along with minor tweaks to other files not listed.
