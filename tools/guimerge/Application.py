#!
# -*- coding: CP1251 -*-
# $Id: Application.py 599 2004-10-04 10:50:52Z gvozdoder $

import os
import glob
import os.path

from Module import *
from ThreadPipe import *

# Обратная связь для получения информации от пользователя
class QueryCallbacks:
	def __init__(self, app):
		self.app = app
	
	def getCommitMessage(self):
		pass

	def getLoginPass(self):
		pass

# Класс, реализующий логику приложения
class Application:
	def __init__(self, scheduler, gui):
		self.currentRealm = ''

		self.gui = gui
		self.scheduler = scheduler
		
		self.tasks = list()
		self.modules = list()
		
		self.loginPassPipe = ThreadPipe()
		self.commitMessagePipe = ThreadPipe()
		
#		self.loginLock = thread.allocate_lock()
#		self.loginReceivedEvent = threading.Event()
		
		self.authValid = False
		self.authUsername = 'dark_lord'
		self.authPassword = 'dark_password'

		self.__addFolders('src')
		self.__selectionChanged()

		self.__createTasks()
#		self.__asyncProcess()

#	def __getLogin(self,realm,user,maysave):
#		self.currentRealm = realm
#		self.loginLock.acquire()
#		try:
#			self.loginReceivedEvent.wait()
#			return (self.authValid, self.authUsername, self.authPassword, False)
#		finally: self.loginLock.release()

#	def __checkStatus(self,module):
#		bad_status = [pysvn.wc_status_kind.unversioned,pysvn.wc_status_kind.conflicted,pysvn.wc_status_kind.missing]
#		
#		client = pysvn.Client()
#		client.callback_get_login = self.__getLogin
#		changes = client.status(module.moduleName, recurse = True, no_ignore = True)
#
#		log = ''
#		status = 'Y'
#
#		for change in changes:
#			if change.text_status == pysvn.wc_status_kind.ignored: continue
#			if change.text_status != pysvn.wc_status_kind.normal and status == 'Y': status = 'N'
#			if change.text_status in bad_status: status = 'X'
#
#			if change.text_status != pysvn.wc_status_kind.normal:
#				log += g_wcStatusKindMap[change.text_status] + ' ' + change.path + '\n'
#
#		return dict(status = status, log = log)

#	def __makeCheckStatusTask(self,module):
#		return self.scheduler.addTask(self.__checkStatus,'status',module)

	def __makeStatusTask(self, module):
		taskdata = dict(module = module, posttask = self.__postRetrieveStatus)
		self.scheduler.addTask(module.retrieveStatus,'status',taskdata)

	def __postRetrieveStatus(self,task):
		module = task.getArguments()['module']
		if task.getResult():
			module.setCommitStatus(task.getResult()['status'])
			module.addToLog(task.getResult()['log'])

	# Создать задачи по получению статуса модулей
	def __createTasks(self):
		for module in self.modules:
			module.setCommitStatus('?')
			self.__makeStatusTask(module)

#	def __queryLogin(self):
#		if self.loginUsername.get(): self.loginUsername.selection_range(0, END)
#		if self.loginPassword.get(): self.loginPassword.selection_range(0, END)
#		self.loginRealm.config(text = self.currentRealm)
#		self.loginForm.deiconify()
#		self.loginUsername.focus_set()

#	def __acceptAuth(self):
#		self.authValid = True
#		self.authUsername = self.loginUsername.get()
#		self.authPassword = self.loginPassword.get()
#		self.loginForm.withdraw()
#		self.focus_set()
#		self.loginReceivedEvent.set()
#		self.loginReceivedEvent.clear()

#	def __rejectAuth(self):
#		self.authValid = False
#		self.loginForm.withdraw()
#		self.loginUsername.focus_set()
#		self.loginReceivedEvent.set()
#		self.loginReceivedEvent.clear()

	def __asyncProcess(self):
		tasks = self.scheduler.extractFinishedTasks()
		
		for task in tasks: task.getArguments()['posttask']()
		self.gui.master.after(100,self.__asyncProcess)
		
#		needUpdateSelection = False
#		for task in self.tasks[:]:
#			if not task.completedLock.locked(): continue
#			if task.name == 'status':
#				needUpdateSelection = True
#				task.data[0].setCommitStatus(task.result['status'])
#				task.data[0].addToLog(task.result['log'])
#				self.tasks.remove(task)
#			elif task.name == 'commit':
#				for k,v in enumerate(task.result):
#					print k.moduleName, v['log'], v['status']
#		
#		if needUpdateSelection: self.__selectionChanged()
#		
#		if self.loginLock.locked() and self.loginForm.state() == 'withdrawn': self.__queryLogin()
#		
#		self.master.after(100,self.__asyncProcess)

	def __processFolder(self,file):
		filename = os.path.normcase(file + '/folder_info.xml')
		if not os.path.exists(filename): return False
		
		module = Module(self.gui,file)
		self.modules.append(module)
#		self.modules.append(ModuleView(self.gui.modulesFrame,file))
#		self.modules[-1].helper.checkBox.config(command = self.__selectionChanged)
		return True

	def __addFolders(self,dirname):
		for dir in glob.glob(dirname + '/*'):
			if os.path.isdir(dir) and not self.__processFolder(dir): self.__addFolders(dir)

#	def __selectAll(self):
#		for module in self.modules: module.helper.selected.set(1)
#		self.__selectionChanged()
		
#	def __deselectAll(self):
#		for module in self.modules: module.helper.selected.set(0)
#		self.__selectionChanged()

	def __selectionChanged(self): pass
#		commitState = 'unknown'
#		
#		for module in self.modules:
#			if not module.helper.selected.get(): continue
#			if module.commitStatus != 'N':
#				commitState = 'disabled'
#				break
#			else: commitState = 'normal'
#		else:
#			if commitState == 'unknown': commitState = 'disabled'
