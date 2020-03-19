#!
# -*- coding: CP1251 -*-
# $Id: ApplicationGui.py 599 2004-10-04 10:50:52Z gvozdoder $

import tkSimpleDialog

from Tkinter import *

g_labelFont = 'courier 12 bold'
g_labelColor = 'blue4'

g_moduleListHeader = [u'��������', u'���� � �����������', u'C', u'M', u'R']

def insertLegendTextValueInfo(text,key,value):
	text.insert(END,key, ('value','valuekey'))
	text.insert(END,value, 'value')

def insertLegendText(text):
	text.config(wrap = 'word')
	
	text.insert(END,u'������� C\n','h1')
	text.insert(END,u'��������� ������� ����� ������.\n', 'desc')
	text.insert(END,u'��������\n','h2')
	insertLegendTextValueInfo(text,u'?', u'\t���������� ��� �� ��������\n')
	insertLegendTextValueInfo(text,u'Y', u'\t������� ����� ������������� ����� � �����������.\n')
	insertLegendTextValueInfo(text,u'N', u'\t� ������� ����� ���� ��������� ��������� � �� ����� ��������� � ������� ������ Commit.\n')
	insertLegendTextValueInfo(text,u'X', u'\t� ������� ����� ���� ���������, ����� �� ������������ � ����������� � �� ������������ � ������� �� ���������� ��� ����, ������������ � ����������� ����������� � ������� �����.\n')
	text.insert(END,u'������� M\n','h1')
	text.insert(END,u'��������� �� ���������� (merge) �� ������� � ����� ������. ���������� � ���� ������� ����� ���� �������� ������ �����, ����� � ������� \'C\' ��������� �������� ', 'desc')
	text.insert(END,u'Y',('desc','valuekey'))
	text.insert(END,u'.\n', 'desc')
	text.insert(END,u'��������\n','h2')
	insertLegendTextValueInfo(text,u'?', u'\t���������� ��� �� ��������\n')
	insertLegendTextValueInfo(text,u'Y', u'\t���������, ����� ������ ������ ������, ��� ������ � �������\n')
	insertLegendTextValueInfo(text,u'N', u'\t�� ���������, ������ � ������� ������������� ����� ����� ������\n')
	text.insert(END,u'������� R\n','h1')
	text.insert(END,u'��������� �� ���������� (merge) �� ����� ������ � ������. ���������� � ���� ������� ����� ���� �������� ������ �����, ����� � ������� \'C\' ��������� �������� ', 'desc')
	text.insert(END,u'Y',('desc','valuekey'))
	text.insert(END,u'.\n', 'desc')
	text.insert(END,u'��������\n','h2')
	insertLegendTextValueInfo(text,u'?', u'\t���������� ��� �� ��������\n')
	insertLegendTextValueInfo(text,u'Y', u'\t���������, ������ � ������� ������, ��� ��� ����� ������\n')
	insertLegendTextValueInfo(text,u'N', u'\t�� ���������, ������ � ������� ������������� ����� ����� ������\n')

	text.tag_config('h1',font = 'ansi 12 bold italic', spacing1 = 15, spacing3 = 5)
	text.tag_config('h2',font = 'ansi 12 bold', lmargin1 = 10, spacing1 = 3, spacing3 = 3)
	text.tag_config('desc',font = 'ansi 10', lmargin1 = 10, lmargin2 = 10)
	text.tag_config('value',font = 'ansi 10', tabs = '50', lmargin1 = 25, lmargin2 = 50)
	text.tag_config('valuekey',font = g_labelFont, foreground = g_labelColor)

# �������������� �������������� �����
class HorizLine(Canvas):
	def __init__(self,parent,**args):
		Canvas.__init__(self,parent,**args)
		self.config(height = 2, highlightthickness = 0)
		
		self.create_line(0,0,self.winfo_width(),0,fill = 'SystemButtonHighlight', tags = 'line')
		self.create_line(0,1,self.winfo_width(),1,fill = 'SystemButtonShadow', tags = 'line')

		self.bind('<Configure>',lambda e: self.__onResize(e))

	def __onResize(self,event):
		for el in self.find_withtag('line'):
			y = self.coords(el)[1]
			self.coords(el, 0, y, event.width, y)

# ������ � ������� �������
class ModuleListRow:
	def __init__(self,frame,texts,header):
		self.parentFrame = frame

		curIndex = self.parentFrame.grid_size()[1]

		self.selected = IntVar()
		self.checkBox = Checkbutton(self.parentFrame, variable = self.selected, state = header and 'disabled' or 'normal', bd = 0)
		self.checkBox.grid(column = 0, row = curIndex, sticky = 'w')
		self.checkBox.config(text = texts[0])
		self.checkBox.config(disabledforeground = self.checkBox.cget('foreground'))
		
		#self.nameLabel = Label(self.parentFrame, text = texts[0], anchor = 'nw')
		self.pathLabel = Label(self.parentFrame, text = texts[1], anchor = 'nw')
		self.commitStatusLabel = Label(self.parentFrame, text = texts[2], anchor = 'nw')
		self.mergeStatusLabel = Label(self.parentFrame, text = texts[3], anchor = 'nw')
		self.newerStatusLabel = Label(self.parentFrame, text = texts[4], anchor = 'nw')

		#self.nameLabel.grid(column = 1, row = curIndex, sticky = 'w')
		self.pathLabel.grid(column = 2, row = curIndex, sticky = 'w')
		self.commitStatusLabel.grid(column = 3, row = curIndex)
		self.mergeStatusLabel.grid(column = 4, row = curIndex)
		self.newerStatusLabel.grid(column = 5, row = curIndex)
		
		if not header:
			self.commitStatusLabel.config(font = g_labelFont, fg = g_labelColor)
			self.mergeStatusLabel.config(font = g_labelFont, fg = g_labelColor)
			self.newerStatusLabel.config(font = g_labelFont, fg = g_labelColor)
			self.showLogButton = Button(self.parentFrame, text = u'���', anchor = 'nw', state = 'disabled')
			self.showLogButton.grid(column = 6, row = curIndex, padx = 5, sticky = 'w')

	def setShowLogCommand(self,fn):
		self.showLogButton['command'] = fn

	def enableShowLogButton(self,enable):
		self.showLogButton['state'] = enable and 'normal' or 'disabled'

# ����� ��� ����������� �������
class LegendForm(Toplevel):
	def __init__(self,parent):
		Toplevel.__init__(self,parent)
		self.transient(parent)
		self.title(u'�������')
		self.parent = parent
		self.__body(self)
		self.protocol("WM_DELETE_WINDOW", self.__close)
		self.focus_set()

	def __body(self,master):
		self.legendCloseButton = Button(master,text = u'�������')
		self.legendScroll = Scrollbar(master)
		self.legend = Text(master, relief = 'flat', bg = 'SystemButtonFace')

		insertLegendText(self.legend)

		self.legendCloseButton.pack(side = 'bottom')
		self.legend.config(state = 'disabled')
		self.legend.pack(expand = 1, fill = 'both')
		
		self.legend.config(yscrollcommand = lambda a,b: self.__setSmartScroll(self.legendCloseButton,self.legendScroll,a,b))
		self.legendCloseButton['command'] = self.__close
		self.legendScroll.config(command = self.legend.yview)

		self.bind('<Return>', self.__close)
		self.bind('<Escape>', self.__close)
		
	def __close(self,event=None):
		self.parent.focus_set()
		self.withdraw()

	def __setSmartScroll(self,aft,scroll,a,b):
		if a == '0' and b == '1':
			scroll.pack_forget()
		else:
			scroll.pack(after = aft, side = 'right', fill = 'y')
		scroll.set(a,b)

# ����� ��� ������� ������ � ������
class LoginForm(tkSimpleDialog.Dialog):
	def __init__(self,master,title,realm,login,passwd):
		self.realm = realm
		self.login = login
		self.passwd = passwd
		tkSimpleDialog.Dialog.__init__(self,master,title)
	
	def body(self,master):
		self.loginRealmDesc = Label(master, text = u'������:', anchor = 'nw')
		self.loginRealm = Label(master, text = u'http://hive/code', anchor = 'nw')
		self.loginUsernameDesc = Label(master, text = u'��� ������������:', anchor = 'nw')
		self.loginPasswordDesc = Label(master, text = u'������:', anchor = 'nw')
		self.loginUsername = Entry(master)
		self.loginPassword = Entry(master, show = '*')

		if self.login:
			self.loginUsername.insert(END,self.login)
			self.loginUsername.selection_range(0, END)
			
		if self.passwd:
			self.loginPassword.insert(END,self.passwd)
			self.loginPassword.selection_range(0, END)

		self.loginRealmDesc.grid(row = 0, column = 0, sticky = E)
		self.loginRealm.grid(row = 0, column = 1, sticky = W)
		self.loginUsernameDesc.grid(row = 1, column = 0, sticky = E)
		self.loginPasswordDesc.grid(row = 2, column = 0, sticky = E)
		self.loginUsername.grid(row = 1, column = 1, columnspan = 2, padx = 2, pady = 2, sticky = N+E+W+S)
		self.loginPassword.grid(row = 2, column = 1, columnspan = 2, padx = 2, pady = 2, sticky = N+E+W+S)

		return self.loginUsername

	def apply(self):
		self.result = (self.loginUsername.get(),self.loginPassword.get())

class ApplicationGui(Frame):
	def __init__(self,master):
		Frame.__init__(self, master)
		
		self.master = master
		self.gridlines = list()
		
		self.pack(expand = 1, fill = 'both', side = 'top')
		self.__createWidgets()

	def __showLegendForm(self):
		if not self.legendWindow: self.legendWindow = LegendForm(self)
		else: self.legendWindow.deiconify()

	def __createWidgets(self):
		self.legendWindow = None
		
		self.buttonFrame = Frame(self)
		self.lineCanvas = HorizLine(self)
		self.actionButtonFrame = Frame(self.buttonFrame)
		self.selectButtonFrame = Frame(self.buttonFrame)
		self.selectLabel = Label(self.selectButtonFrame, text = u'���������:', anchor = 'nw')
		self.actionLabel = Label(self.actionButtonFrame, text = u'��������:', anchor = 'nw')

		self.buttonFrame.pack(side = 'bottom', fill = 'x')
		self.lineCanvas.pack(side = 'bottom', fill = 'x', padx = 2, pady = 1)
		self.actionButtonFrame.pack(side = 'top', anchor = 'w', fill = 'both')
		self.selectButtonFrame.pack(side = 'top', anchor = 'w', fill = 'both')
		self.selectLabel.pack(side = 'left', anchor = 'w')
		self.actionLabel.pack(side = 'left', anchor = 'w')
		
		self.quitButton = Button(self.buttonFrame, anchor = 's', text = u'�����', command = self.quit)
		self.legendButton = Button(self.buttonFrame, anchor = 's', text = u'�������', command = self.__showLegendForm)
		self.selectAllButton = Button(self.selectButtonFrame, anchor = 's', text = u'�������� ���')
		self.deselectAllButton = Button(self.selectButtonFrame, anchor = 's', text = u'����� ���������')
		self.mergeToProjectButton = Button(self.actionButtonFrame, anchor = 's', text = u'Merge � ������')
		self.mergeFromProjectButton = Button(self.actionButtonFrame, anchor = 's', text = u'Merge �� �������')
#		self.selectAllButton['command'] = self.__selectAll
#		self.deselectAllButton['command'] = self.__deselectAll
#		self.mergeToProjectButton['command'] = self.__commit
#		self.mergeFromProjectButton['command'] = self.__commit

		self.quitButton.pack(side = 'left', anchor = 's', expand = 1, fill = 'x', padx = 2, pady = 2)
		self.legendButton.pack(side = 'left', anchor = 's', padx = 2, pady = 2)
		self.selectAllButton.pack(side = 'left', expand = 1, fill = 'x', anchor = 's', padx = 2, pady = 2)
		self.deselectAllButton.pack(side = 'left', expand = 1, fill = 'x', anchor = 's', padx = 2, pady = 2)
		self.mergeToProjectButton.pack(side = 'left', expand = 1, fill = 'x', anchor = 's', padx = 2, pady = 2)
		self.mergeFromProjectButton.pack(side = 'left', expand = 1, fill = 'x', anchor = 's', padx = 2, pady = 2)
		
		self.infoFrame = Frame(self)
		self.infoFrame.pack(side = 'top')
		self.infoFrameLabel = Label(self.infoFrame, text = u'��������� �������', borderwidth = 1)
		self.infoFrameLabel.pack()

		self.modulesFrame = Frame(self)
		self.modulesFrame.pack(expand = 0, fill = 'both', side = 'top', anchor = 'nw')
		self.modulesFrame.columnconfigure(2,weight=1)
		
		self.modulesHeader = ModuleListRow(self.modulesFrame, g_moduleListHeader, True)

		self.lineCanvas = HorizLine(self.modulesFrame)
		self.lineCanvas.grid(columnspan = 100, padx = 2, pady = 1, sticky = W+E+N+S)

	def createModuleListRow(self,texts):
		self.gridlines.append(ModuleListRow(self.modulesFrame, texts, False))
		return self.gridlines[-1]

	def getLoginPass(self,realm):
		self.update()
		dialog = LoginForm(self,u'�����','test','test','test')
		return dialog.result
