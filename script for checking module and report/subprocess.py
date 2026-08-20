import subprocess


def capture_output_text():
    
    #1. If we set capture_output equale to True, the stdout and stderr will be set to PIPE, which means the all of them will
    # be transfered to python by PIPE. And if we don't set capture_output or stderr, or stdout to True, python will not capture
    # the message. Instead, the terminal capture it.
    #2. Set capture_output or stderr, stdout to a certain value in subprocess.run. It is esentially redirecting to other place.
    # By deafult, a place where store stdout and stderr is terminal
    p1 = subprocess.run(['ls','-lh'],capture_output = True, text = True)
    
    
    print(p1.args)
    print(p1.returncode)
    print(p1.stdout) #saved by bytes
    print(p1.stdout.decode()) # decode stdout to string
    print(p1.stdout) # If set text equal to True, output stdout with string, rather than with bytes.


    


def stdout_to_file():


    #1. with.....as..... is a python inside statement, which can automatically manage resource like file,
    # it can automatically close the file when leave that block.
    #2. open() is a python inside function, its first parameter is file, if it doesn't exist, will create and name it. 
    # the second parameter means the way to open the file, here is write mode, which can erase the contents of a file if it exists contents.
    #3. f is the file object here
    #4. (python)p1.stdout can not capture the stdout as stdout is set to f(file), rather than subprocess.PIPE
    with open("output.txt", 'w') as f:
        p1 = subprocess.run(['ls', '-lh'], stdout = f, text = True)
        f.write("HelloWorld!")


def stderr_check():
    
    #1.If you don't set shell equal to True, instead it's False. Python will treat entire string as a program, like "ls -lh",
    # obviouslly, it can't find this program. So we can pass in list as runs first parameter, python will automatically treat
    # its first parameter as command, the second as option, and the third as parameter like file name.
    # If we set shell equal to True, we don't have to pass in a list, instead we can as a whole string as the string will be
    # run in the shell. That means if shell equal to True, python subprocess.run will start a shell where the command will be
    # performed here. 
    # But be careful to set shell to True due to some vicious operations which are not obvious.
    
    #2.PIPE is a attributite from subprocess.

    #3.If we set check equal to True, python will capture the exception from subprocess by their returncode value.
    p1 = subprocess.run(['ls', '-lh', 'file1.txt'], text = True, stderr = subprocess.PIPE,stdout = subprocess.PIPE,check = True)
    #print(p1.stdout)
    
    if p1.returncode == 0:
        print("Run normally.", p1.stdout)
    else:
        print("Return code:",p1.returncode)
        print(p1.stderr)    


def stderr_to_DEVNULL():
    

    #python can't capture stderr, as stderr is gonna be set to /dev/null which is a character device that recive input, then output none.
    p1 = subprocess.run(['ls', '-lh','file1.txt'], stderr = subprocess.DEVNULL)

    print(p1.stderr)
    

def input_from_another_output():

   
    
    with open("test1.txt","w") as f:
        f.write("This")
        f.write("is")
        f.write("a")
        f.write("test file")

    #1. p2 command receive the input of the p1 command's output.And, need to know p1.stdout should be matched with p2.Here are
    # all strings.
    p1 = subprocess.run(['cat', 'test1.txt'],text = True, capture_output= True)

    p2 = subprocess.run(['grep','-n','test'],text=True, stdout = subprocess.PIPE, input = p1.stdout)

    print(p2.stdout)    


def main():
    
    #capture_output_text()   
    
    #stdout_to_file()
    
    #stderr_check()
     
    #stderr_to_DEVNULL()

    input_from_another_output()
if __name__ == "__main__":
    main()