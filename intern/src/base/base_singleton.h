//
//  base_singleton.h
//  base
//
//  Created by Tobias Schwandt on 09/03/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#define BASE_SINGLETON_FUNC(_Classname)         \
    public:                                     \
    static _Classname& GetInstance()            \
    {                                           \
        static _Classname s_Instance;           \
        return s_Instance;                      \
    }