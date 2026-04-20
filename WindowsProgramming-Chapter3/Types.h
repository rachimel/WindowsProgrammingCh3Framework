#pragma once
template<typename Ty>
concept Num = std::is_arithmetic_v<Ty>;
