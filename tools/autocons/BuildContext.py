#
# $Id: BuildContext.py 599 2004-10-04 10:50:52Z gvozdoder $

# Build context
class BuildContext:
	def __init__(self,builder,entity):
		self.m_builder = builder
		self.m_entity = entity

	def build(self):
		self.m_entity.build()

	def getConfig(self):
		return self.m_entity.m_prjconfig

	def isStatic(self,name = None):
		if name == None: name = self.m_entity.m_project.m_name
		return self.m_entity.m_slnconfig['libconf'][name] == 'lib'

	def getSolutionConfig(self):
		return self.m_entity.m_slnconfig
