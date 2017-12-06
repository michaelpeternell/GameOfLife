//
//  gol-config.h
//  GameOfLife
//
//  Created by Michael Peternell on 06.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#ifndef GameOfLife_gol_config_h
#define GameOfLife_gol_config_h

// This config file should be included from all Source files.

// Visual C++ doesn't like the fopen()-function (or fscanf() or a bunch of other C methods).
// It throws this warning when we use it:
//
//   warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
//   1>c:\program files(x86)\windows kits\10\include\10.0.15063.0\ucrt\stdio.h(207) : note: Siehe Deklaration von "fopen"
//
// I think fopen() is perfectly safe. We're grown up, be silent VC++ !!
#define _CRT_SECURE_NO_WARNINGS 1

#ifdef _WIN32
#define GOL_WIN_ONLY 1
#define GOL_MAC_ONLY 0
#else
#define GOL_WIN_ONLY 0
#define GOL_MAC_ONLY 1
#endif

#define USE_OPENMP GOL_WIN_ONLY

#endif // GameOfLife_gol_config_h
