// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("niLang");

::gZip <- ::CreateGlobalInstance("niLang.Zip").QueryInterface("iZip");
::LINT_CHECK_TYPE("interface_def<iZip>", ::gZip);
