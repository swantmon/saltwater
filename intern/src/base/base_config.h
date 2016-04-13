//
//  base_config.h
//  base
//
//  Created by Tobias Schwandt on 09/03/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

// -----------------------------------------------------------------------------
// If condition
// -----------------------------------------------------------------------------
#define BASE_TRUE 1
#define BASE_FALSE 0

// -----------------------------------------------------------------------------
// Use default namespace or user defined namespace.
// -----------------------------------------------------------------------------
#define BASE_HAS_USER_NAMESPACE         BASE_TRUE

// -----------------------------------------------------------------------------
// In case of a user defined namespace define its identifier.
// -----------------------------------------------------------------------------
#define BASE_USER_NAMESPACE             Base