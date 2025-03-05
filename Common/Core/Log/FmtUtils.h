#pragma once
// Define fmt formatters for custom types here:

namespace fmt {

    // TODO: Do we need to inherit from the string formatter?
    template<typename T>
    struct formatter<NCL::Maths::TVector3<T>> : formatter<std::string> {
        auto format(const NCL::Maths::TVector3<T>& p, format_context& ctx) const {
            return format_to(ctx.out(), "({: 5.2f}, {: 5.2f}, {: 5.2f})", p.x, p.y, p.z);
        }
    };

    template<>
    struct formatter<NCL::Maths::Vector4> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }
        template<typename FormatContext>
        auto format(const NCL::Maths::Vector4& p, FormatContext& ctx) {
            return format_to(ctx.out(), "({: 5.2f}, {: 5.2f}, {: 5.2f}, {: 5.2f})", p.x, p.y, p.z, p.w);
        }
    };
}

// TODO: Can't get ostream to compile, though we don't really need it
//namespace fmt {
//	template <typename T> 
//	struct formatter<NCL::Maths::TVector3<T>> : ostream_formatter {};
//}