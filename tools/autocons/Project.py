#
# $Id: Project.py 664 2004-10-08 10:46:36Z gvozdoder $

import re
import string
import os.path

from ProjectConfiguration import *
from Exceptions import *

# xml related imports
import xml.dom.minidom
from xml import xpath

# Project class
class Project:
	def __init__(self, sln, prjFile, prjName, prjType, prjId, prjDeps, prjCfgs, fullcfgs):
		if not os.path.exists(prjFile): raise "Project file [" + prjFile + "] not found"
		self.m_solution = sln
		self.m_id = prjId
		self.m_file = prjFile
		self.m_path = os.path.dirname(self.m_file)
		self.m_name = prjName
		self.m_type = prjType
		self.m_deps = prjDeps
		self.m_cfgs = prjCfgs
		self.m_cfgFiles = dict()
		self.m_configs = dict()
		self.__parseProject(fullcfgs)

	def __parseProject(self,fullcfgs):
		# Read contents of project file
		project = xml.dom.minidom.parse(self.m_file)
		configs = xpath.Evaluate('VisualStudioProject/Configurations/Configuration', project)
		
		cfgmap = dict()

		for config in configs:
			name = string.lower(re.sub(r'\|.*','',str(config.getAttribute('Name'))))
#			if name not in self.m_cfgs: continue
			cfgmap[name] = config
#			try: self.m_configs[name] = ProjectConfiguration(self,project,config,name)
#			except UnsupportedConfigurationTypeException: pass

		for cfgname, cfg in fullcfgs.iteritems():
			realname = cfgname
			while realname not in cfgmap: realname = fullcfgs[realname]['copyFrom']
			
			try: self.m_configs[cfgname] = ProjectConfiguration(self,project,cfgmap[realname],cfgname)
			except UnsupportedConfigurationTypeException: pass

#		for cfgname, cfg in fullcfgs.iteritems():
#			if cfgname not in self.m_configs:
#				try: self.m_configs[cfgname] = ProjectConfiguration(self,project,cfgmap[cfg['copyFrom']],cfgname)
#				except UnsupportedConfigurationTypeException: pass
