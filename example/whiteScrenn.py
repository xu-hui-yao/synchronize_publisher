import tkinter as tk
root = tk.Tk()
root.attributes('-fullscreen', True)
root.configure(background='white')
root.title("White Screen")
def exit_fullscreen(event):
    root.attributes('-fullscreen', False)
    root.destroy()
root.bind("<Escape>", exit_fullscreen)
root.mainloop()