#include <iostream>
#include <iomanip>

#include "janssonxx.h"

using namespace std;

#define ASSERT_OP(lhs, rhs, op, m) \
	do { \
		if(!((lhs) op (rhs))) { \
			std::cerr << std::boolalpha; \
			std::cerr << __FILE__ << '[' << __LINE__ << "]: ERROR: " << (m) << std::endl; \
			std::cerr << "\ttest:   " << #lhs << ' ' << #op << ' ' << #rhs << std::endl; \
			std::cerr << "\tresult: " << (lhs) << ' ' << #op << ' ' << (rhs) << std::endl; \
			return 1; \
		} \
	} while(0)
#define ASSERT_EQ(lhs, rhs, m) ASSERT_OP(lhs, rhs, ==, m)
#define ASSERT_NE(lhs, rhs, m) ASSERT_OP(lhs, rhs, !=, m)
#define ASSERT_TRUE(p, m) ASSERT_OP(p, true, ==, m)
#define ASSERT_FALSE(p, m) ASSERT_OP(p, true, !=, m)

int main() {
	jansson::Value e1(jansson::Value::load_file("test.json"));
	jansson::Value e2(e1);
	jansson::Value e3;
	jansson::Value e4(jansson::Value::load_string("{\"foo\": true, \"bar\": \"test\"}"));

	ASSERT_TRUE(e1.is_object(), "e1 is not an object");
	ASSERT_TRUE(e2.is_object(), "e2 is not an object");
	ASSERT_TRUE(e3.is_undefined(), "e3 has a defined value");
	ASSERT_TRUE(e4.is_object(), "e4 is not an object");

	ASSERT_EQ(e1.size(), 1, "e1 has too many properties");
	ASSERT_EQ(e2.size(), 1, "e2 has too many properties");
	ASSERT_EQ(e4.size(), 2, "e4 does not have 2 elements");

	ASSERT_TRUE(e1.get("web-app").is_object(), "e1[0].web-app is not an object");
	ASSERT_EQ(e1.get("web-app").get("servlet").at(0).get("servlet-class").as_string(), "org.cofax.cds.CDSServlet", "property has incorrect value");
	ASSERT_EQ(e1["web-app"]["servlet"][0]["servlet-class"].as_string(), "org.cofax.cds.CDSServlet", "property has incorrect value");

	ASSERT_EQ(e4["foo"].as_boolean(), true, "property has incorrect value");

	jansson::Iterator i(e1.get("web-app"));
	ASSERT_EQ(i.key(), "taglib", "first iterator result has incorrect key");
	i.next();
	ASSERT_EQ(i.key(), "servlet", "first iterator result has incorrect key");
	i.next();
	ASSERT_EQ(i.key(), "servlet-mapping", "first iterator result has incorrect key");
	i.next();
	ASSERT_FALSE(i.valid(), "iterator has more values than expected");

	e3 = 12.34;
	ASSERT_TRUE(e3.is_number(), "e3 is not a number after assignment");
	ASSERT_EQ(e3.as_real(), 12.34, "e3 has incorrect value after assignment");

	e3 = true;
	ASSERT_TRUE(e3.is_boolean(), "e3 is not a boolean after assignment");
	ASSERT_EQ(e3.as_boolean(), true, "e3 has incorrect value after assignment");

	e3 = "foobar";
	ASSERT_TRUE(e3.is_string(), "e3 is not a string after assignment");
	ASSERT_EQ(e3.as_string(), "foobar", "e3 has incorrect value after assignment");

	return 0;
}
