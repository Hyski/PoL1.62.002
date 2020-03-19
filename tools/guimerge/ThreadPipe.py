#!
# $Id: ThreadPipe.py 599 2004-10-04 10:50:52Z gvozdoder $

import thread

class Signal:
	def __init__(self):
		self.__accessLock = thread.allocate_lock()
		self.__flagChangedLock = thread.allocate_lock()
		self.__flagChangedLock.acquire()
		self.__flag = False
	
	def wait(self):
		flag = False
		while not flag:
			self.__accessLock.acquire()
			try:
				flag = self.__flag
				if flag: self.__flag = False
			finally: self.__accessLock.release()
			
			if not flag: self.__flagChangedLock.acquire()

	def wait0(self):
		flag = False

		self.__accessLock.acquire()
		try:
			flag = self.__flag
			if flag: self.__flag = False
			return flag
		finally: self.__accessLock.release()

	def set(self):
		flagChanged = False
		self.__accessLock.acquire()
		try:
			flagChanged = not self.__flag
			self.__flag = True
		finally: self.__accessLock.release()
		if flagChanged: self.__flagChangedLock.release()

# Класс для обмена данными между потоками
class ThreadPipe:
	def __init__(self):
		self.__querySignal = Signal()
		self.__queryRepliedSignal = Signal()

	def query(self):
		self.__querySignal.set()
		self.__queryRepliedSignal.wait()
		return self.__result

	def hasQuery(self):
		return self.__querySignal.wait0()

	def reply(self,result = None):
		self.__result = result
		self.__queryRepliedSignal.set()

		