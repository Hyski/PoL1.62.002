import updater

files = ['antlr.dll', 'boost_regex.dll',
		 'mll_core.dll', 'mll_fs.dll',
		 'muffle.dll', 'mutual.dll',
		 'stlport.dll', 'storage.dll',
		 'vis.dll', 'vis_mesh.dll',
		 'zlib.dll', 'polmis.exe',
		 'antlr.pdb', 'boost_regex.pdb',
		 'mll_core.pdb', 'mll_fs.pdb',
		 'muffle.pdb', 'mutual.pdb',
		 'stlport.pdb', 'storage.pdb',
		 'vis.pdb', 'vis_mesh.pdb',
		 'zlib.pdb', 'polmis.pdb']

updater.makeUpdate('executable/bin/debug', files, '_debug')


