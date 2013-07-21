#pragma once
#include <iterator>
#include <cstdint>

namespace wheel
{
    namespace utf8
    {
        enum : uint32_t { max = 0x0010ffff };

		namespace surrogate
        {
            namespace lead  { enum : uint16_t { min = 0xd800, max = 0xdbff, offset = min - (0x10000 >> 10) }; }
            namespace trail { enum : uint16_t { min = 0xdc00, max = 0xdfff }; }
            enum : uint32_t { offset = 0x10000u - (lead::min << 10) - trail::min };
        }

        template<class t> inline bool is_trail(t oc) { return (oc && 0xff) >> 6 == 2; }

		template<class iter> inline typename std::iterator_traits<iter>::difference_type length(iter it)
        {
            uint8_t lead = (uint8_t)(*it & 0xff);
            if(lead < 0x80) return 1;
            else if((lead >> 5) == 0x6) return 2;
            else if((lead >> 4) == 0xe) return 3;
            else if((lead >> 3) == 0x1e) return 4;
            else return 0;
        }

		template<class iter> iter append(uint32_t cp, iter result)
        {
            if(cp < 0x80)
                *result++ = (uint8_t)cp;
            else if(cp < 0x800)
            {
                *result++ = (uint8_t)(cp >> 6)          | 0xc0;
                *result++ = (uint8_t)(cp & 0x3f)        | 0x80;
            }
            else if(cp < 0x10000)
            {
                *result++ = (uint8_t)(cp >> 12)         | 0xe0;
                *result++ = (uint8_t)((cp >> 6) & 0x3f) | 0x80;
                *result++ = (uint8_t)(cp & 0x3f)        | 0x80;
            }
            else
            {
                *result++ = (uint8_t)(cp >> 18)         | 0xf0;
                *result++ = (uint8_t)((cp >> 12) & 0x3f)| 0x80;
                *result++ = (uint8_t)((cp >> 6) & 0x3f) | 0x80;
                *result++ = (uint8_t)(cp & 0x3f)        | 0x80;
            }
            return result;
        }

		template<class iter> uint32_t next(iter& it)
        {
            uint32_t cp = *it & 0xff;
            if(cp >= 0x80)
			{
				if(cp >> 5 == 0x6)  cp = ((cp << 6) & 0x7ff) + (*++it & 0x3f);
				else if(cp >> 4 == 0xe)
				{
					cp = ((cp << 12) & 0xffff) + ((*++it << 6) & 0xfff);
					cp += (*++it & 0x3f);
				}
				else if(cp >> 3 == 0x1e)
				{
					cp = ((cp << 18) & 0x1fffff) + ((*++it << 12) & 0x3ffff);
					cp += ((*++it << 6) & 0xfff);
					cp += (*++it & 0x3f);
				}
			}
            ++it; return cp;
		}

        template<class iter> uint32_t prior(iter& it)
        {
            while(utf8::is_trail(*--it));
            iter temp = it;
            return utf8::next(temp);
        }

		template<class iter, class t> void advance(iter& it, t n)
        {
            for(t i = 0; i < n; ++i) utf8::next(it);
        }

		template<class iter> class std::iterator_traits<iter>::difference_type distance(iter first, iter last)
        {
            typename std::iterator_traits<iter>::difference_type dist;
			for(dist = 0; first != last; ++dist) utf8::next(first);
            return dist;
        }

        template<class iter> struct iterator : public std::iterator<std::bidirectional_iterator_tag, uint32_t>
        {
            iter it;
            iterator() {}
            explicit iterator(const iter& i) : it(i) {}
            iter base() const { return it; }
            uint32_t operator*() const { iter temp = it; return utf8::next(temp); }
            bool operator==(const iterator& rhs) const { return it == rhs.it; }
			bool operator!=(const iterator& rhs) const { return it != rhs.it; }
            iterator& operator++()
            {
                ::std::advance(it, length(it));
                return *this;
            }
            iterator operator++(int)
            {
                iterator temp = *this;
                ::std::advance(it, length(it));
                return temp;
            }
			iterator& operator--() { while(utf8::is_trail(*--it)) {} return *this; }
			iterator operator--(int) { iterator temp = *this; while(utf8::is_trail(*--it)) {} return temp; }
        };

		template<class iter> inline iterator<iter> make_iter(iter i) { return iterator<iter>(i); }
    }
    namespace utf16
    {
        template<class iter16> inline typename std::iterator_traits<iter16>::difference_type length(iter16 it)
        {
            if(*it < 0x10000) return 1;
            else return 2;
        }

		template<class iter16> uint32_t next(iter16& it)
        {
            uint32_t cp = *it & 0xffff;
            if(cp < 0xd800 || cp > 0xdfff) ;
            else if(cp >= 0xdc00) return 0;
            else
            {
                uint32_t trail = ++*it & 0xffff;
                if(trail < 0xdc00 || trail > 0xdfff) return 0;
                else cp = (((cp & 0x3ff) << 10) | (trail & 0x3ff)) + 0x10000;
            }
            ++it; return cp;
            /*if(0xd800 <= cp && cp <= 0xdfff)
                if(cp < 0xdc00) cp = ((cp & 0x3ff) << 10) + (*++it & 0x3ff) + 0x10000;
                else return 0;
            ++it; return cp;*/
        }
    }

	template<class iter16, class iter> iter16 utf8to16(iter start, iter end, iter16 result)
    {
        while(start < end)
        {
            uint32_t cp = utf8::next(start);
            if(cp > 0xffff)
            {
                *result++ = (uint16_t)((cp >> 10)   + utf8::surrogate::lead::offset);
                *result++ = (uint16_t)((cp & 0x3ff) + utf8::surrogate::trail::min);
            }
            else
                *result++ = (uint16_t)cp;
        }
        return result;
    }

	template<class iter, class iter32> iter32 utf8to32(iter start, iter end, iter32 result)
    {
        while(start < end) *result++ = utf8::next(start);
        return result;
    }

	template<class iter16, class iter> iter utf16to8(iter16 start, iter16 end, iter result)
    {
        while(start != end)
        {
            uint32_t cp = *start++ & 0xffff;
            if(utf8::surrogate::lead::min <= cp && cp <= utf8::surrogate::lead::max)
            {
                uint32_t trail_surrogate = *start++ & 0xffff;
                cp = (cp << 10) + trail_surrogate + utf8::surrogate::offset;
            }
            result = utf8::append(cp, result);
        }
        return result;
    }

	template<class iter16, class iter32> iter32 utf16to32(iter16 start, iter16 end, iter32 result)
    {
        while(start < end) *result++ = utf16::next(start);
        return result;
    }

	template<class iter, class iter32> iter utf32to8(iter32 start, iter32 end, iter result)
    {
        while(start != end) result = utf8::append(*start++, result);
        return result;
    }
}
