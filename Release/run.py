#import
from tkinter import *
try:
    from tkinter.ttk import *
except ImportError:
    print("import ttk error")
from tkinter.dnd import *
#comment
print('Hello World')
#loop
for i in range(1,5):
    for j in range(1,5):
        print(i,"x",j,"=",i*j)
else:
    print("the for loop is over")
#func
def hello():
    global count
    print("click count ",count)
    count+=1
count=0
#new win
win = Tk()
win.title("hello Tkiner")
win.geometry("400x200")

#3 button bind hello func
btn=Button(win,text="button1",command=hello)
btn.pack()
btn=Button(win,text="button2",command=hello)
btn.pack()
btn=Button(win,text="button3",command=hello)
btn.pack()

mainloop()
#raw_input();