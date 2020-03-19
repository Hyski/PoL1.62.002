#
# $Id: Solution.py 664 2004-10-08 10:46:36Z gvozdoder $

import re
import string
import os.path

from Project import Project

g_slnProjects = re.compile(r'project\("(\{.*?\})"\) = "(.*?)", "(.*?)", "(\{.*?\})"(.*?)endproject\b', re.DOTALL)
g_slnProjectDependencies = re.compile(r'projectsection\(projectdependencies\).*?\n(.*?)endprojectsection', re.DOTALL)
g_slnProjectDependencyItems = re.compile(r'(\{.*?\}) = \{.*?\}', re.DOTALL)
g_slnGlobalSection = re.compile(r'\bglobal\b(.*?)\bendglobal\b', re.DOTALL)
g_slnSolutionConfigurations = re.compile(r'globalsection\(solutionconfiguration\).*?\n(.*?)endglobalsection', re.DOTALL)
g_slnSolutionConfigurationItems = re.compile(r'(\w*?) = \w*', re.DOTALL)
g_slnProjectConfigurations = re.compile(r'globalsection\(projectconfiguration\).*?\n(.*?)endglobalsection', re.DOTALL)
g_slnProjectConfigurationItems = re.compile(r'(\{\S*?\})\.(\w*?)\.build\.0 = (.*?)\|win32', re.DOTALL)

# Solution class
class Solution:
	def __init__(self,slnName,fullcfgs):
		if not os.path.exists(slnName): raise "Solution file [" + slnName + "] not found"
		self.m_file = slnName
		self.m_path = os.path.dirname(self.m_file)
		self.m_projects = dict()
		self.m_configurations = list()
		self.__parseSolution(fullcfgs)

	def testAllPresent(self, fullconfigs):
		for k in fullconfigs.iterkeys():
			if k not in self.m_configurations: raise "Configuration " + k + " is not found in solution"

	def __parseSolution(self,fullcfgs):
		# Read contents of solution file
		contents = string.lower(file(self.m_file).read())
		# Find global solution section
		slnglobal = g_slnGlobalSection.findall(contents)[0]
		# Find solution configurations
		self.m_configurations = g_slnSolutionConfigurationItems.findall(g_slnSolutionConfigurations.findall(slnglobal)[0])
		# Find all projects
		projects = g_slnProjects.findall(contents)
		# Find project configurations
		prjconfigs = g_slnProjectConfigurationItems.findall(g_slnProjectConfigurations.findall(slnglobal)[0])
		
		idtoname = dict()
		
		for prj in projects: idtoname[prj[3]] = prj[1]
		
		# Create projects
		for prj in projects:
			cfgs = dict()
			for cfg in prjconfigs:
				if cfg[0] == prj[3]: cfgs[cfg[2]] = cfg[1]
			
			for cfgname, cfg in fullcfgs.iteritems():
				if cfgname not in cfgs and 'copyFrom' in cfg:
					cfgs[cfgname] = cfgs[cfg['copyFrom']]
			
			deps = []
			for dep in g_slnProjectDependencyItems.findall(g_slnProjectDependencies.findall(prj[4])[0]):
				deps.append(idtoname[dep])
			
			self.m_projects[prj[3]] = Project(self,prj[2],prj[1],prj[0],prj[3],deps,cfgs,fullcfgs)
