#!
# -*- coding: CP1251 -*-
# $Id: guimerge.pyw 599 2004-10-04 10:50:52Z gvozdoder $

from Scheduler import *
from Application import *
from ApplicationGui import *

tk = Tk()
app = Application(Scheduler(),ApplicationGui(tk))
tk.title(u'Синхронизация проектов')
app.gui.mainloop()

