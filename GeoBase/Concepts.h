#pragma once
template <typename Ty>
concept RealNumber = std::is_arithmetic_v<Ty>;

template <typename T,typename U>
concept isCommonNumberType = std::is_integral_v<T> && std::is_integral_v<U>
|| std::is_floating_point_v<T> && std::is_floating_point_v<U>;