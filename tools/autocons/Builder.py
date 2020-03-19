#
# $Id: Builder.py 664 2004-10-08 10:46:36Z gvozdoder $

import string
import os.path

from BuildEntity import *
from BuildContext import *

# Builds solution
class Builder:
	def __init__(self,solution,env):
		self.m_sln = solution
		self.m_env = env

	def buildSolution(self,configs):
		#self.m_sln.testAllPresent(configs)
		for k, v in configs.iteritems():
#			if k not in self.m_sln.m_configurations:
#				if 'copyFrom' in v and v['copyFrom'] in configs:
#					self.__buildConfiguration(k,configs[v['copyFrom']])
#				else:
#					raise 'Unknown configuration: ' + k
#			else:
			self.__buildConfiguration(k,v)

	def __buildConfiguration(self,slncfg,cfg):
		for prjname, prj in self.m_sln.m_projects.iteritems(): self.__buildProject(slncfg,cfg,prj)

	def __buildProject(self,slncfg,cfg,prj):
		if slncfg not in prj.m_cfgs: return
		
		prjcfg = prj.m_cfgs[slncfg]
		cconf = prj.m_configs[slncfg] #[prjcfg]
		
		src_dir = os.path.normcase('#' + prj.m_path)
		dest_dir = os.path.normcase(cconf.m_intDir)
		
		entity = BuildEntity(self.m_env, self.m_sln, prj, cconf, cfg, slncfg)
		context = BuildContext(self, entity)

		needdup = cconf.needDuplicate()
		
		self.m_env.BuildDir(dest_dir, src_dir, duplicate = needdup)
		sconscript = os.path.normcase(dest_dir+'/'+cconf.m_sconscript)
		self.m_env.SConscript(sconscript, exports='context')
		results = entity.getResult()

		self.m_env.Alias(string.lower(slncfg), results)
		