// Copyright 2022 Gerard Gaudeau
// Distributed under MIT license
// See file LICENSE for detail at https://github.com/ggaudeau/anyjson/blob/main/LICENSE

#pragma once

#include <iostream>	// std::cout
#include <utility>	// std::forward

void print() {
}

#if defined(NDEBUG)

void println() {
}

template<typename Elm, typename ...Args>
void print(Elm&&, Args&& ...) {
}

template<typename Elm, typename ...Args>
void println(Elm&&, Args&& ...) {
}

#else

void println() {
  std::cout << std::endl;
}

template<typename Elm, typename ...Args>
void print(Elm&& elm, Args&& ...args) {
  std::cout << std::forward<Elm>(elm);
  print(std::forward<Args>(args)...);
}

template<typename Elm, typename ...Args>
void println(Elm&& elm, Args&& ...args) {
  std::cout << std::forward<Elm>(elm);
  println(std::forward<Args>(args)...);
}

#endif
