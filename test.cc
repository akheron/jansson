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

	jansson::Value e5(jansson::Value::from(12.34));
	ASSERT_TRUE(e5.is_number(), "e5 is not a number after assignment");
	ASSERT_EQ(e5.as_real(), 12.34, "e5 has incorrect value after assignment");

	jansson::Value e6(jansson::Value::from(true));
	ASSERT_TRUE(e6.is_boolean(), "e6 is not a boolean after assignment");
	ASSERT_EQ(e6.as_boolean(), true, "e6 has incorrect value after assignment");

	jansson::Value e7(jansson::Value::from("foobar"));
	ASSERT_TRUE(e7.is_string(), "e7 is not a string after assignment");
	ASSERT_EQ(e7.as_string(), "foobar", "e7 has incorrect value after assignment");

	jansson::Value e8(jansson::Value::object());
	ASSERT_TRUE(e8.is_object(), "e8 is not an object after assignment");

	jansson::Value e9(jansson::Value::null());
	ASSERT_TRUE(e9.is_null(), "e9 is not null after assignment");

	jansson::Value e10(jansson::Value::array());
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");

	e10.set_at(0, jansson::Value::from("foobar"));
	ASSERT_EQ(e10.size(), 1, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[0].as_string(), "foobar", "e10[0] has incorrect value after assignment");

	e10.set_at(1, jansson::Value::from("foobar"));
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[1].as_string(), "foobar", "e10[0] has incorrect value after assignment");

	e10.set_at(0, jansson::Value::from("barfoo"));
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[0].as_string(), "barfoo", "e10[0] has incorrect value after assignment");

	e10.set_at(100, jansson::Value::null());
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");

	e10.insert_at(1, jansson::Value::from("new"));
	ASSERT_EQ(e10.size(), 3, "e10 has incorrect size after insert");
	ASSERT_EQ(e10[1].as_string(), "new", "e10[1] has incorrect value after insert");
	ASSERT_EQ(e10[2].as_string(), "foobar", "e10[2] has incorrect value after insert");

	e10.del_at(0);
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect size after delete");
	ASSERT_EQ(e10[1].as_string(), "foobar", "e10[1] has incorrect value after delete");

	e10.clear();
	ASSERT_EQ(e10.size(), 0, "e10 has incorrect number of elements after clear");

	jansson::Value e11(jansson::Value::object());
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");

	e11.set_key("foo", jansson::Value::from("test"));
	ASSERT_EQ(e11.size(), 1, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["foo"].as_string(), "test", "e11.foo has incorrect value after assignment");

	e11.set_key("foo", jansson::Value::from("again"));
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");
	ASSERT_EQ(e11.size(), 1, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["foo"].as_string(), "again", "e11.foo has incorrect value after assignment");

	e11.set_key("bar", jansson::Value::from("test"));
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");
	ASSERT_EQ(e11.size(), 2, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["bar"].as_string(), "test", "e11.foo has incorrect value after assignment");

	e11.clear();
	ASSERT_EQ(e11.size(), 0, "e11 has incorrect number of properties after clear");

	jansson::Value e12(jansson::Value::object());
	e12.set_key("foo", jansson::Value::from("test"));
	e12.set_key("bar", jansson::Value::from(3));
	char* out_cstr = e12.save_string(0);
	string out(out_cstr);
	free(out_cstr);
	ASSERT_EQ(out, "{\"bar\": 3,\"foo\": \"test\"}\n", "object did not serialize as expected");

	std::istringstream instr(out);
	instr >> e12;
	ASSERT_TRUE(e12.is_object(), "e12 is not an object after stream read");
	ASSERT_EQ(e12.size(), 2, "e12 has wrong size after stream read");
	ASSERT_EQ(e12.get("bar").as_integer(), 3, "e12.bar has incorrect value after stream read");
	ASSERT_EQ(e12.get("foo").as_string(), "test", "ee12.test has incorrect value after stream read");

	std::ostringstream outstr;
	outstr << e12;
	ASSERT_EQ(instr.str(), "{\"bar\": 3,\"foo\": \"test\"}\n", "object did not serialize as expected");

	return 0;
}
