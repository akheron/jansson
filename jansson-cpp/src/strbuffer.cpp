/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "strbuffer.hpp"

// Implementation moved to header file since we're using std::string
// which handles memory management automatically. The header contains
// all the necessary inline implementations.

// This file is kept for build system compatibility
