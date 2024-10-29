// #pragma once

// #include <odb/core.hxx>
// #include <boost/date_time/gregorian/gregorian.hpp>
// #include <string>


// namespace odb {
//   namespace pgsql {
//     // Specialize odb::value_traits for boost::gregorian::date
//     template<>
//     struct value_traits<boost::gregorian::date, id_date>
//     {
//       typedef boost::gregorian::date value_type;
//       typedef std::string query_type;
//       typedef std::string image_type;

//       static void set_image(image_type& img, bool& null, const value_type& val)
//       {
//         if (val.is_not_a_date())
//         {
//           null = true;
//         }
//         else
//         {
//           img = boost::gregorian::to_iso_extended_string(val);
//           null = false;
//         }
//       }

//       static value_type set_value(const image_type& img, bool is_null)
//       {
//         if (is_null || img.empty())
//         {
//           return boost::gregorian::date(boost::gregorian::not_a_date_time);
//         }
//         else
//         {
//           return boost::gregorian::from_string(img);
//         }
//       }
//     };
//   }
// }
