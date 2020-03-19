#
# $Id: BuildEntity.py 686 2004-10-10 00:25:25Z gvozdoder $

import re
import os.path
import CustomBuilder

g_standardLibrary = dict( dll = '/MD', mt = '/MT', st = '/ML' )

g_defaultLibraries = ['kernel32.lib', 'user32.lib', 'gdi32.lib', 'winspool.lib', 'comdlg32.lib', \
					  'advapi32.lib', 'shell32.lib', 'ole32.lib', 'oleaut32.lib', 'uuid.lib', 'odbc32.lib', 'odbccp32.lib']

def removeDups(lst):
	result = dict()
	for item in lst: result[item] = None
	return result.keys()

# Builds project
class BuildEntity:
	def __init__(self,env,solution,project,prjconfig,slnconfig,slncfgname):
		self.m_env = env
		self.m_solution = solution
		self.m_project = project
		self.m_prjconfig = prjconfig
		self.m_slnconfig = slnconfig
		self.m_slncfgname = slncfgname
		self.m_result = None

	def __getTempPdbName(self):
		return os.path.normcase(self.__getTempPath() + 'vc71.pdb')

	def __getTempIdbName(self):
		return os.path.normcase(self.__getTempPath() + 'vc71.idb')

	def __getPchName(self):
		return os.path.normcase(self.__getTempPath() + 'precomp.pch')

	def __getPdbName(self):
		return os.path.normcase(self.__getTempPath() + self.m_project.m_name + '.pdb')
	
	def __getTempPath(self):
		return os.path.normcase('temp/' + self.m_project.m_name + '/' + self.m_slncfgname + '/')

	def __getProjectPath(self):
		return os.path.normcase(self.m_project.m_path + '/')

	def __prepareCompileEnvironment(self,env,prjconfig):
		cenv = env.Copy()
		cenv.Append(CPPPATH = prjconfig['AdditionalIncludeDirectories'])

		defines = prjconfig['PreprocessorDefinitions']
		defines.extend(self.m_slnconfig['globalDefinesInclude'])
		
		for define in defines[:]:
			if define in self.m_slnconfig['globalDefinesExclude']: defines.remove(define)

		defines = removeDups(defines)
		cenv.Append(CPPDEFINES = defines)

		cxxflags = list()
		cxxflags += prjconfig['Optimization']
		cxxflags += prjconfig['BasicRuntimeChecks']
		if self.m_slnconfig['debug']: cxxflags += [g_standardLibrary[self.m_slnconfig['crt']] + 'd']
		else: cxxflags += [g_standardLibrary[self.m_slnconfig['crt']]]
		if self.m_slnconfig['homeVersion']: cxxflags += prjconfig['MinimalRebuild']
		cxxflags += prjconfig['ForceConformanceInForLoopScope']
		cxxflags += ['/Zi', '/EHsc']
		tpdbname = self.__getTempPdbName()
		pchname = self.__getPchName()
		cxxflags += ['/Fd' + tpdbname]

		if prjconfig['UsePrecompiledHeader'] == 'auto':
			cxxflags += ['/YX']
		elif prjconfig['UsePrecompiledHeader'] == 'use':
			cxxflags += ['/Yu' + prjconfig['PrecompiledHeaderThrough']]
			cxxflags += ['/Fp' + pchname]
		elif prjconfig['UsePrecompiledHeader'] == 'create':
			cxxflags += ['/Yc' + prjconfig['PrecompiledHeaderThrough']]
			cxxflags += ['/Fp' + pchname]

		cenv.Append(CCFLAGS = cxxflags)
		cenv['PDB'] = '#' + self.__getPdbName()
		cenv['CCPDBFLAGS'] = ['']
		
		for forceinc in prjconfig['ForcedIncludeFiles']:
			cenv.Append(CCFLAGS = ['/FI' + forceinc])

		for diswarn in prjconfig['DisableSpecificWarnings']:
			cenv.Append(CCFLAGS = ['/wd' + diswarn])

		return cenv

	def __prepareEnvironment(self):
		cenv = self.m_env.Copy()
		CustomBuilder.createBuilder(cenv)

		# Make paths and defines
		cenv.Append(LIBPATH = self.m_prjconfig.m_props['AdditionalLibraryDirectories'])
		cenv.Append(LIBPATH = os.path.normcase('#lib/' + self.m_slncfgname))
		
		# Default Visual Studio libraries
		cenv.Append(LIBS = g_defaultLibraries)
		
		tpdbname = self.__getPdbName()
		
		linkflags = ['/DEBUG', '/PDB:' + tpdbname]
		if not self.m_slnconfig['homeVersion']: linkflags += ['/INCREMENTAL:NO']
		cenv.Append(LINKFLAGS = linkflags)

		# Project dependencies		
		cenv.Append(LIBS = self.m_prjconfig.m_props['AdditionalDependencies'])
		
		return cenv

	def __buildSourceFile(self,file,env,fileconfig):
		cenv = self.__prepareCompileEnvironment(env,fileconfig)
		objname = re.match(r'(.*)\..*$',file).group(1)
		objname = os.path.normcase(objname)
		result = cenv.SharedObject(target = objname, source = file)
		
		if fileconfig['UsePrecompiledHeader'] == 'use':
			cenv.Depends(result,'precomp.pch')
		elif fileconfig['UsePrecompiledHeader'] == 'create':
			cenv.SideEffect('precomp.pch',result)
			
		return result

	def __buildResourceFile(self,file,env,fileconfig):
		cenv = self.__prepareCompileEnvironment(env,fileconfig)
		objname = re.match(r'(.*)\..*$',file).group(1)
		objname = os.path.normcase(objname)
		result = cenv.RES(target = objname, source = file)
		
		return result

	def __buildGrammarFile(self,file,env,fileconfig):
		gpath = os.path.dirname(file)
		
		targets = list()
		for trg in fileconfig['Outputs']:
			if len(gpath): targets.append(os.path.normcase(gpath + '/' + trg))
			else: targets.append(trg)

		result = env.Command(targets,file, 'src\\external\\antlr\\tool\\cantlr.exe -o ${TARGET.dir} $SOURCE')
		
		return result

	def __initiateBuild(self,cenv):
		binpath = os.path.normcase('#executable/bin/' + self.m_slncfgname + '/')
		libpath = os.path.normcase('#lib/' + self.m_slncfgname + '/')
		tmppath = os.path.normcase('#temp/' + self.m_project.m_name + '/' + self.m_slncfgname + '/')

		libname = os.path.normcase(self.m_project.m_name + '.lib')
		pdbname = os.path.normcase(self.m_project.m_name + '.pdb')

		objects = list()

		for file,fileconfig in self.m_prjconfig.m_files.iteritems():
			if re.search(r'(\.cpp|\.c)$',file) != None:
				objects.extend(self.__buildSourceFile(file,cenv,fileconfig))
			elif re.search(r'\.rc$',file) != None:
				objects.extend(self.__buildResourceFile(file,cenv,fileconfig))
			elif re.search(r'\.g$',file) != None:
				self.__buildGrammarFile(file,cenv,fileconfig)

		intermed = None
		msgarbage = ['#' + self.__getTempPdbName(),
					 '#' + self.__getTempIdbName(),
					 '#' + self.__getPdbName(),
					 os.path.normcase('#' + self.__getTempPath() + self.m_project.m_name + '.ilk'),
					 ]

		prjtype = self.m_prjconfig.m_type
		if self.m_project.m_name in self.m_slnconfig['libconf']: prjtype = self.m_slnconfig['libconf'][self.m_project.m_name]

		cenv.Precious(msgarbage)

		if prjtype == 'dll':
			dllname = os.path.normcase(self.m_project.m_name + '.dll')
			intermed = cenv.SharedLibrary(target = [dllname] + msgarbage, source = objects)
			result = [cenv.Install(binpath,[pdbname,dllname]),cenv.Install(libpath,libname)]
		elif prjtype == 'exe':
			exename = os.path.normcase(self.m_project.m_name + '.exe')
			intermed = cenv.Program(target = [exename] + msgarbage, source = objects)
			result = cenv.Install(binpath,[pdbname,exename])
		elif prjtype == 'lib':
			intermed = cenv.StaticLibrary(target = [libname] + msgarbage, source = objects)
			result = cenv.Install(libpath,libname)
		else: pass

		for dep in self.m_project.m_deps:
			cenv.Depends(intermed, os.path.normcase('#lib/' + self.m_slncfgname + '/' + dep + '.lib'))

		return result

	def __buildProject(self):
		cenv = self.__prepareEnvironment()
		return self.__initiateBuild(cenv)

	def build(self):
		self.m_result = self.__buildProject()

	def getResult(self):
		return self.m_result