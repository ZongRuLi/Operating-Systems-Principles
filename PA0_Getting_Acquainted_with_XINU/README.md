# PA 0: Getting Acquainted with XINU

## 1. Objective
The objective of this introductory lab is to familiarize you with the process of compiling and running XINU, the tools
involved, and the run-time environment and segment layout

## 4. Tasks
You will be using the csc501-lab0.tgz you have downloaded and compiled by following the lab setup guide. And you are
asked to write several XINU functions that perform the following tasks:
1. long zfunction(long param)<br>
Clear from the 10th to 18th bits (counting left to right, the left most bit is the 0th bit), shift the parameter param by 4 bits to
the left, and then fill the right most bits with 0. For example, the input parameter 0xaabbccdd should generate a return
value of 0xa800cdd0. You can assume that the size of long is 4 bytes. The code for this function should be entirely written in
x86 assembly. You should not use inline assembly, (i.e., do not use asm(???)). To investigate the assembly code generated
by the compiler, you can use the tool objdump -d <___.o> to disassemble an object file. The object files reside in
the /compile directory within the main Xinu directory. You can also see some of the *.S files in the /sys directory for
reference.<br>

(Tips: please check out the 2nd and the 3rd readings above before doing this question.)<br>

2. void printsegaddress()<br>

Print the addresses indicating the end of the text, data, and BSS segments of the Xinu OS. Also print the 4-byte contents
(in hexadecimal) preceding and after those addresses. This function can be written in C.<br>

3. void printtos()<br>

Print the address of the top of the run-time stack for whichever process you are currently in, right before and right after you
get into the printtos() function call. In addition, print the contents of upto four stack locations below the top of the stack
(the four or fewer items that have been the most recently pushed, if any). Remember that stack elements are 32 bits wide,
and be careful to perform pointer arithmetic correctly. Also note that there are local variables and arguments on the stack,
among other things. See the hints given for #4 below, especially on stacktrace.c and proc.h. Your function can be written
entirely in C, or you can use in-line assembly if you prefer.<br>

4. void printprocstks(int priority)<br>

For each existing process with larger priority than the parameter, print the stack base, stack size, stacklimit, and stack
pointer. Also, for each process, include the process name, the process id and the process priority.
To help you do this, please look into proc.h in the h/ directory. Note the proctab[] array that holds all processes. Also,
note that the pesp member of the pentry structure holds the saved stack pointer. Therefore, the currently executing
process has a stack pointer that is different from the value of this variable. In order to help you get the stack pointer of the
currently executing process, carefully study the stacktrace.c file in the sys/ directory. The register %esp holds the current
stack pointer. You can use in-line assembly(i.e., asm("...")) to do this part.<br>

5. void printsyscallsummary()<br>

Print the summary of the system calls which have been invoked for each process. This task is loosely based on the
functionality of LTTng (http://lttng.org/) . There are 43 system calls declared. Please look into kernel.h in the h/ directory
to see all declared system calls. However, only 27 system calls are implemented in this XINU version. The implementation
of these 27 system calls are in the sys/ directory. You are asked to print the frequency (how many times each system call
type is invoked) and the average execution time (how long it takes to execute each system call type in average) of these 27
system calls for each process. In order to do this, you will need to modify the implementation of these 27 types of system
calls to trace them whenever they are invoked. To measure the time, XINU provides a global variable named ctr1000 to
track the time (in milliseconds) passed by since the system starts. Please look into sys/clkinit.c and sys/clkint.S to
see the details.<br>

You will also need to implement two other functions:<br>

void syscallsummary_start(): to start tracing the system calls. All the system calls are invoked after calling this function
(and before calling syscallsummary_stop()) will be presented in the system call summary.<br>

void syscallsummary_stop(): to stop tracing the system calls.<br>

In other words, these two functions determine the duration in which the system calls are traced.<br>

To help you complete this task, we provide two files, syscalls.txt (https://moodlecourses2223.wolfware.ncsu.edu/pluginfile.php/1237267/mod_assign/introattachment/0/syscalls.txt?forcedownload=1)
lists all the system calls you will need to trace, and test.c (https://moodlecourses2223.wolfware.ncsu.edu/pluginfile.php/1237267/mod_assign/introattachment/0/test.c?forcedownload=1)
demonstrates the usage of the functions you will implement (note that this is only the test file and will not be used for
grading).<br>

Implement this lab as a set of functions that can be called from main(). Each function should reside in a separate file in the
sys directory, and should be incorporated into the Makefile. The files should be named after the functions they are
implementing with C files having the .c extension and the assembly files having the .S extension. For example, the file that
will hold void printsegaddress() should be named printsegaddress.c; and the file that will hold long zfunction(long
param) should be named zfunction.S. You should put syscallsummary_start, syscallsummary_stop functions in the
same file as printsyscallsummary function and name it as printsyscallsummary.c . If you require a header file, please
name it lab0.h. Note: as you create new files, you may need to update the Makefile (located in the compile/directory) to
configure it to compile your files correctly. Just look at what is done for the existing files (e.g., main.c) to see what you
have to do.

## 5. Additional Questions
Write your answers to the following questions in a file named Lab0Answers.txt (in simple text).
Please place this file in the sys/ directory and turn it in, along with the above programming assignment.<br>

1. Assuming the XINU text begins at address 0x0, draw a rough diagram of XINU’s memory layout with addresses
derived from your experimental measurements. Include the information you uncovered from running your version
of printsegaddress() and printprocstks().
2. What is the difference in stack top address before and after calling printtos() ? Draw a diagram to illustrate what
are the contents of the items pushed into the stack between these two time points.

3. Which byte order is adopted in the host machine that we are using ? How did you find out ?

4. Briefly describe the mov, push, pusha, pop, and popa instructions in the x86.

5. In a stack frame, local variables are stored below the top of the stack. In task 3, does your result show all the local
variables declared in your printtos function? If not, can you explain that? (hint: try to disable the compiler optimization
by specifying-O0 in your Makefile)


