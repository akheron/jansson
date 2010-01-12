#include <iostream>
#include <iomanip>
#include <malloc.h>

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

	e3 = jansson::Value::from(12.34);
	ASSERT_TRUE(e3.is_number(), "e3 is not a number after assignment");
	ASSERT_EQ(e3.as_real(), 12.34, "e3 has incorrect value after assignment");

	e3 = jansson::Value::from(true);
	ASSERT_TRUE(e3.is_boolean(), "e3 is not a boolean after assignment");
	ASSERT_EQ(e3.as_boolean(), true, "e3 has incorrect value after assignment");

	e3 = jansson::Value::from("foobar");
	ASSERT_TRUE(e3.is_string(), "e3 is not a string after assignment");
	ASSERT_EQ(e3.as_string(), "foobar", "e3 has incorrect value after assignment");

	e3 = jansson::Value::object();
	ASSERT_TRUE(e3.is_object(), "e3 is not an object after assignment");

	e3 = jansson::Value::null();
	ASSERT_TRUE(e3.is_null(), "e3 is not null after assignment");

	e3.set(0, jansson::Value::from("foobar"));
	ASSERT_TRUE(e3.is_array(), "e3 is not an array after index assignment");
	ASSERT_EQ(e3.size(), 1, "e3 has incorrect number of elements after assignment");
	ASSERT_EQ(e3[0].as_string(), "foobar", "e3[0] has incorrect value after assignment");

	e3.set(1, jansson::Value::from("foobar"));
	ASSERT_TRUE(e3.is_array(), "e3 is not an array after index assignment");
	ASSERT_EQ(e3.size(), 2, "e3 has incorrect number of elements after assignment");
	ASSERT_EQ(e3[1].as_string(), "foobar", "e3[0] has incorrect value after assignment");

	e3.set(0, jansson::Value::from("barfoo"));
	ASSERT_TRUE(e3.is_array(), "e3 is not an array after index assignment");
	ASSERT_EQ(e3.size(), 2, "e3 has incorrect number of elements after assignment");
	ASSERT_EQ(e3[0].as_string(), "barfoo", "e3[0] has incorrect value after assignment");

	e3.set(100, jansson::Value::null());
	ASSERT_TRUE(e3.is_array(), "e3 is not an array after index assignment");
	ASSERT_EQ(e3.size(), 2, "e3 has incorrect number of elements after assignment");

	e3.clear();
	ASSERT_EQ(e3.size(), 0, "e3 has incorrect number of elements after clear");

	e3.set("foo", jansson::Value::from("test"));
	ASSERT_TRUE(e3.is_object(), "e3 is not an object after property assignment");
	ASSERT_EQ(e3.size(), 1, "e3 has incorrect number of properties after assignment");
	ASSERT_EQ(e3["foo"].as_string(), "test", "e3.foo has incorrect value after assignment");

	e3.set("foo", jansson::Value::from("again"));
	ASSERT_TRUE(e3.is_object(), "e3 is not an object after property assignment");
	ASSERT_EQ(e3.size(), 1, "e3 has incorrect number of properties after assignment");
	ASSERT_EQ(e3["foo"].as_string(), "again", "e3.foo has incorrect value after assignment");

	e3.set("bar", jansson::Value::from("test"));
	ASSERT_TRUE(e3.is_object(), "e3 is not an object after property assignment");
	ASSERT_EQ(e3.size(), 2, "e3 has incorrect number of properties after assignment");
	ASSERT_EQ(e3["bar"].as_string(), "test", "e3.foo has incorrect value after assignment");

	e3.clear();
	ASSERT_EQ(e3.size(), 0, "e3 has incorrect number of properties after clear");

	e3 = jansson::Value::object();
	e3.set("foo", jansson::Value::from("test"));
	e3.set("bar", jansson::Value::from(3));
	char* out_cstr = e3.save_string(0);
	string out(out_cstr);
	free(out_cstr);
	ASSERT_EQ(out, "{\"bar\": 3,\"foo\": \"test\"}\n", "object did not serialize as expected");

	return 0;
}
