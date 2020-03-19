#
# $Id: ProjectConfiguration.py 1602 2005-02-06 14:02:03Z gvozdoder $

import re
import copy
import string
import os.path

# xml related imports
import xml.dom.minidom
from xml import xpath

g_prjUsePrecompMap = { '0' : 'disable', '1' : 'create', '2' :'auto', '3' : 'use' }
g_prjOptimizationMap = { '0' : ['/Od'], '1' : ['/Os'], '2' : ['/Ot'], '3' : ['/Ox'] }
g_prjRuntimeChecksMap = { '0' : [], '1' : ['/RTCs'], '2' : ['/RTCu'], '3' : ['/RTCsu'] }
g_prjMinimalRebuildMap = { 'TRUE' : ['/Gm'], 'FALSE' : [] }
g_prjIncrementalLinkMap = { '1' : ['/INCREMENTAL:NO'], '2' : ['/INCREMENTAL'] }
g_prjForConformanceMap = { 'TRUE' : ['/Zc:forScope'], 'FALSE' : [] }

g_prjConfigTypeMap = { '1' : 'exe', '2' : 'dll', '4' : 'lib' }

def readAttrib(props,compiler,name,fn):
	val = compiler.getAttribute(name)
	if val != '': props[name] = fn(str(val))

def substPath(str,pattern,replacement):
	if len(replacement) == 0: return string.replace(str,pattern,'')
	else: return string.replace(str,pattern,replacement+'/')

def processFileName(name,cfg):
	result = name
	if result[0] == '"' and result[-1] == '"': result = result[1:-1]
	if '$(IntDir)' in result: result = substPath(result,'$(IntDir)', cfg.m_intDir)
	if '$(ProjectDir)' in result: result = substPath(result,'$(ProjectDir)', cfg.m_project.m_path)
	if '$(ProjectName)' in result: result = string.replace(result, '$(ProjectName)', cfg.m_project.m_name)
	if '$(SolutionDir)' in result: result = substPath(result,'$(SolutionDir)', cfg.m_project.m_solution.m_path)
	if '$(ConfigurationName)' in result: result = string.replace(result, '$(ConfigurationName)', cfg.m_solutionName)
	if not os.path.isabs(result): result = '#' + result
	return result

def processFileNameList(names,cfg):
	nnames = list()
	for name in string.split(names,';'): nnames.append(processFileName(name,cfg))
	return string.join(nnames,';')

# Project's configuration
class ProjectConfiguration:
	def __init__(self,prj,doc,domel,slncfg):
		type = str(domel.getAttribute('ConfigurationType'))
		if type not in g_prjConfigTypeMap: raise UnsupportedConfigurationTypeException()

		self.m_slncfg = slncfg
		self.m_project = prj
		self.m_props = dict()
		self.m_files = dict()
		self.m_sconscript = None

		self.m_needDuplicate = 0

		self.__makeDefaultConfiguration()
		self.__parseConfiguration(doc,domel)
		if self.m_sconscript == None: self.m_sconscript = self.m_project.m_path + '/SConscript'


	def needDuplicate(self): return self.m_needDuplicate

	def setType(self,type):
		self.m_type = type

	def __makeDefaultProps(self,props):
		props['AdditionalIncludeDirectories'] = []
		props['PreprocessorDefinitions'] = []
		props['UsePrecompiledHeader'] = 'disabled'
		props['PrecompiledHeaderThrough'] = ''
		props['PrecompiledHeaderFile'] = ''
		props['Optimization'] = ['/O2']
		props['BasicRuntimeChecks'] = []
		props['MinimalRebuild'] = []
		props['ForceConformanceInForLoopScope'] = []
		props['ForcedIncludeFiles'] = []
		props['DisableSpecificWarnings'] = []
		
		props['AdditionalLibraryDirectories'] = []
		props['LinkIncremental'] = ['/INCREMENTAL:NO']
		props['AdditionalDependencies'] = []

		props['Outputs'] = []
		
		props['MlsHasCustomBuild'] = False
		props['MlsDependsOn'] = []
		
		props['ResAdditionalIncludeDirectories'] = []

	def __makeDefaultConfiguration(self):
		self.__makeDefaultProps(self.m_props)

	def __trimTrailingSlashes(self,props,name):
		result = list()

		for dir in props[name]:
			if dir[-1] in ['\\','/']: result.append(dir[:-1])
			else: result.append(dir)

		props[name] = result

	def __readProps(self,props,el):
		compiler = xpath.Evaluate('Tool[@Name="VCCLCompilerTool"]',el)
		if len(compiler) > 1: raise 'Multiple compiler tools found in ' + self.m_project.m_name
		elif len(compiler) == 1:
			compiler = compiler[0]
			readAttrib(props,compiler,'AdditionalIncludeDirectories',lambda n: string.split(processFileNameList(n,self),';'))
			readAttrib(props,compiler,'PreprocessorDefinitions',lambda n: string.split(n,';'))
			readAttrib(props,compiler,'UsePrecompiledHeader',lambda n: g_prjUsePrecompMap[n])
			readAttrib(props,compiler,'PrecompiledHeaderThrough',lambda n: n)
			readAttrib(props,compiler,'PrecompiledHeaderFile',lambda n: processFileName(n,self))
			readAttrib(props,compiler,'Optimization',lambda n: g_prjOptimizationMap[n])
			readAttrib(props,compiler,'BasicRuntimeChecks',lambda n: g_prjRuntimeChecksMap[n])
			readAttrib(props,compiler,'MinimalRebuild',lambda n: g_prjMinimalRebuildMap[n])
			readAttrib(props,compiler,'ForceConformanceInForLoopScope',lambda n: g_prjForConformanceMap[n])
			readAttrib(props,compiler,'ForcedIncludeFiles', lambda n: string.split(n,';'))
			readAttrib(props,compiler,'DisableSpecificWarnings', lambda n: string.split(n,';'))

		linker = xpath.Evaluate('Tool[@Name="VCLinkerTool"]',el)
		if len(linker) == 0: linker = xpath.Evaluate('Tool[@Name="VCLibrarianTool"]',el)
		
		if len(linker) > 1: raise 'Multiple linker or librarian tools found in ' + self.m_project.m_name
		elif len(linker) == 1:
			linker = linker[0]
			readAttrib(props,linker,'AdditionalLibraryDirectories',lambda n: string.split(processFileNameList(n,self),';'))
			readAttrib(props,linker,'LinkIncremental',lambda n: g_prjIncrementalLinkMap[n])
			readAttrib(props,linker,'AdditionalDependencies',lambda n: string.split(n))

		custombuild = xpath.Evaluate('Tool[@Name="VCCustomBuildTool"]',el)
		
		if len(custombuild) > 1: raise 'Multiple custom build tools found in ' + self.m_project.m_name
		elif len(custombuild) == 1:
			custombuild = custombuild[0]
			props['MlsHasCustomBuild'] = True
			readAttrib(props,custombuild,'CommandLine',lambda n: n)
			readAttrib(props,custombuild,'Outputs',lambda n: string.split(n,';'))
			props['Outputs'] = [t for t in props['Outputs'] if len(t)]

		restool = xpath.Evaluate('Tool[@Name="VCResourceCompilerTool"]',el)
		if len(restool) > 1: raise 'Multiple resource compiler tools found in ' + self.m_project.m_name
		elif len(restool) == 1:
			restool = restool[0]
			tprops = dict()
			readAttrib(tprops,restool,'AdditionalIncludeDirectories',lambda n: string.split(processFileNameList(n,self),';'))
			if 'AdditionalIncludeDirectories' in tprops: props['ResAdditionalIncludeDirectories'] = tprops['AdditionalIncludeDirectories']

		self.__trimTrailingSlashes(props,'AdditionalIncludeDirectories')
		self.__trimTrailingSlashes(props,'AdditionalLibraryDirectories')
		self.__trimTrailingSlashes(props,'ResAdditionalIncludeDirectories')

		for i,aid in enumerate(props['AdditionalIncludeDirectories']):
			if aid[-1] == '\\': props['AdditionalIncludeDirectories'][i] = aid[:-1]
		for i,aid in enumerate(props['AdditionalLibraryDirectories']):
			if aid[-1] == '\\': props['AdditionalLibraryDirectories'][i] = aid[:-1]
		for i,aid in enumerate(props['ResAdditionalIncludeDirectories']):
			if aid[-1] == '\\': props['ResAdditionalIncludeDirectories'][i] = aid[:-1]

	def __addFile(self,file):
		name = os.path.normpath(str(file.getAttribute('RelativePath')))
		if re.search(r'(\.cpp|\.c|\.g|\.sconscript|\.rc)$',name) == None: return

		# If we have a grammar file then we need to duplicate files using BuildDir cmd
		if re.search(r'\.g$',name) != None: self.m_needDuplicate = 1

		configs = xpath.Evaluate('FileConfiguration',file)
		props = copy.deepcopy(self.m_props)
		for config in configs:
			cfgname = string.lower(re.sub(r'\|.*','',str(config.getAttribute('Name'))))
			if cfgname != self.m_name: continue
			exclude = str(config.getAttribute('ExcludedFromBuild'))
			if exclude == 'TRUE': return
			self.__readProps(props,config)

		if '.sconscript' in name:
			if self.m_sconscript != None: raise 'Multiple sconscripts in ' + self.m_project.m_name
			self.m_sconscript = name
		else:
			self.m_files[name] = props

	def __parseConfiguration(self,doc,domel):
		self.m_type = g_prjConfigTypeMap[str(domel.getAttribute('ConfigurationType'))]
		self.m_name = str(string.lower(re.sub(r'\|.*','',domel.getAttribute('Name'))))
		self.m_solutionName = self.m_slncfg #self.m_project.m_cfgs[self.m_name]
		self.m_intDir = processFileName('temp/' + self.m_project.m_name + '/' + self.m_solutionName,self)
		
		self.__readProps(self.m_props,domel)

		# Remove all references to lib\<something> dir from AdditionalLibraryDirectories
		adlibdirs = []
		for adlibdir in self.m_props['AdditionalLibraryDirectories']:
			if not re.match(r'#lib\\[^\\]*\\?$',os.path.normcase(adlibdir)): adlibdirs.append(adlibdir)
		self.m_props['AdditionalLibraryDirectories'] = adlibdirs

		files = xpath.Evaluate('//File', doc)
		for file in files: self.__addFile(file)
		
		for fname,fprops in self.m_files.iteritems():
			for foutput in fprops['Outputs']:
				if foutput in self.m_files:
					self.m_files[foutput]['MlsDependsOn'].append(fname)
