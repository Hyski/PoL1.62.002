#!
# $Id: Module.py 599 2004-10-04 10:50:52Z gvozdoder $

import re
import pysvn

# xml related imports
import xml.dom.minidom
from xml import xpath

g_cutSrc = re.compile(r'^src[/\\](.*)')

g_wcStatusKindMap = {
	pysvn.wc_status_kind.added: 'A',
	pysvn.wc_status_kind.conflicted: 'C',
	pysvn.wc_status_kind.deleted: 'D',
	pysvn.wc_status_kind.external: 'X',
	pysvn.wc_status_kind.ignored: 'I',
	pysvn.wc_status_kind.incomplete: '!',
	pysvn.wc_status_kind.missing: '!',
	pysvn.wc_status_kind.merged: 'G',
	pysvn.wc_status_kind.modified: 'M',
	pysvn.wc_status_kind.none: ' ',
	pysvn.wc_status_kind.normal: ' ',
	pysvn.wc_status_kind.obstructed: '~',
	pysvn.wc_status_kind.replaced: 'R',
	pysvn.wc_status_kind.unversioned: '?',
}

class Module:
	def __init__(self,gui,path):
		self.appgui = gui
		
		self.log = ''
		self.commitStatus = '?'
		self.path = path
		self.shortName = g_cutSrc.findall(self.path)[0]
		self.folderInfoFile = self.path + '/folder_info.xml'
		
		folderinfo = xml.dom.minidom.parse(self.folderInfoFile)
		svnpath = xpath.Evaluate('folder-info/original-url', folderinfo)

		if len(svnpath) > 1: raise 'Multiple original urls found in ' + self.folderInfoFile
		elif len(svnpath) == 1:
			if len(svnpath[0].childNodes) != 1: raise 'Malformed ' + self.folderInfoFile
			self.svnpath = svnpath[0].childNodes[0].nodeValue

		self.gui = gui.createModuleListRow([unicode(self.shortName),str(self.svnpath),u'?',u'?', u'?'])
		self.gui.setShowLogCommand(self.__showLog)

	def __showLog(self):
		print self.log

	def addToLog(self,text):
		self.log += text
		self.gui.enableShowLogButton(self.log)

	def setCommitStatus(self,status):
		self.commitStatus = status
		self.gui.commitStatusLabel.config(text = self.commitStatus)

	def retrieveStatus(self):
		bad_status = [pysvn.wc_status_kind.unversioned,pysvn.wc_status_kind.conflicted,pysvn.wc_status_kind.missing]
		
		client = pysvn.Client()
#		client.callback_get_login = self.__getLogin

		try:
			changes = client.status(self.path, recurse = True, no_ignore = True)
	
			log = ''
			status = 'Y'
	
			for change in changes:
				if change.text_status == pysvn.wc_status_kind.ignored: continue
				if change.text_status != pysvn.wc_status_kind.normal and status == 'Y': status = 'N'
				if change.text_status in bad_status: status = 'X'
	
				if change.text_status != pysvn.wc_status_kind.normal:
					log += g_wcStatusKindMap[change.text_status] + ' ' + change.path + '\n'
					
			return dict(status = status, log = log)
		except: pass
			

