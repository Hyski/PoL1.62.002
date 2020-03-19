#!
# $Id: Scheduler.py 599 2004-10-04 10:50:52Z gvozdoder $

import thread
import threading

from ThreadPipe import *

# Задача
class SchedulerTask:
	def __init__(self, task, name, *args):
		self.__task = task
		self.__finishedLock = thread.allocate_lock()
		self.__data = args
		self.__result = None
		self.__name = name

	def getTask(self): return self.__task
	def isFinished(self): return self.__finishedLock.locked()
	def getTaskArguments(self): return self.__data
	def getResult(self): return self.__result
	def getName(self): return self.__name
	def taskExecute(self):
		task.result = task.__task(*task.data)
		task.completedLock.acquire()

# Управляет задачами, выполняющимися в отдельном потоке
class Scheduler:
	def __init__(self):
		self.__tasks = list()
		self.__finishedTasks = list()
		self.__tasksLock = thread.allocate_lock()
		self.__tasksChanged = Signal()
		self.__thread = thread.start_new_thread(self.__threadFunc, ())

	def addTask(self, fn, name, *args):
		task = SchedulerTask(fn,name,*args)
		self.__operateOnTaskList(lambda: self.__tasks.append(task))
		self.__tasksChanged.set()
		return task

	def extractFinishedTasks(self):
		self.__tasksLock.acquire()
		try:
			result = self.__finishedTasks[:]
			self.__finishedTasks = []
			return result
		finally: self.__tasksLock.release()

	def __operateOnTaskList(self,fn):
		self.__tasksLock.acquire()
		try: return fn()
		finally: self.__tasksLock.release()

	def __getTaskOperation(self):
		if self.__tasks: return self.__tasks[0]
		return None

	def __finishTask(self,task):
		self.__tasks.pop(0)
		self.__finishedTasks.append(task)

	def __threadFunc(self):
		while True:
			task = self.__operateOnTaskList(lambda: len(self.__tasks) and self.__tasks[0] or None)
			if task:
				task.execute()
				self.__operateOnTaskList(lambda: self.__finishTask(task))
			else:
				self.__tasksChanged.wait()
