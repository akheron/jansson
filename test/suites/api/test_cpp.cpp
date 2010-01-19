#include <iostream>
#include <iomanip>
#include <malloc.h>

#include "jansson.hpp"

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

int json_cpp_tests() {
	json::Value e1(json::load_file("test.json"));
	json::Value e2(e1);
	json::Value e3;
	json::Value e4(json::load_string("{\"foo\": true, \"bar\": \"test\"}"));

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

	json::Iterator i(e1.get("web-app"));
	ASSERT_EQ(i.key(), "taglib", "first iterator result has incorrect key");
	i.next();
	ASSERT_EQ(i.key(), "servlet", "first iterator result has incorrect key");
	i.next();
	ASSERT_EQ(i.key(), "servlet-mapping", "first iterator result has incorrect key");
	i.next();
	ASSERT_FALSE(i.valid(), "iterator has more values than expected");

	json::Value e5(json::Value(12.34));
	ASSERT_TRUE(e5.is_number(), "e5 is not a number after assignment");
	ASSERT_EQ(e5.as_real(), 12.34, "e5 has incorrect value after assignment");

	json::Value e6(json::Value(true));
	ASSERT_TRUE(e6.is_boolean(), "e6 is not a boolean after assignment");
	ASSERT_EQ(e6.as_boolean(), true, "e6 has incorrect value after assignment");

	json::Value e7(json::Value("foobar"));
	ASSERT_TRUE(e7.is_string(), "e7 is not a string after assignment");
	ASSERT_EQ(e7.as_string(), "foobar", "e7 has incorrect value after assignment");

	json::Value e8(json::object());
	ASSERT_TRUE(e8.is_object(), "e8 is not an object after assignment");

	json::Value e9(json::null());
	ASSERT_TRUE(e9.is_null(), "e9 is not null after assignment");

	json::Value e10(json::array());
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");

	e10.set_at(0, json::Value("foobar"));
	ASSERT_EQ(e10.size(), 1, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[0].as_string(), "foobar", "e10[0] has incorrect value after assignment");

	e10.set_at(1, json::Value("foobar"));
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[1].as_string(), "foobar", "e10[0] has incorrect value after assignment");

	e10.set_at(0, json::Value("barfoo"));
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");
	ASSERT_EQ(e10[0].as_string(), "barfoo", "e10[0] has incorrect value after assignment");

	e10.set_at(100, json::null());
	ASSERT_TRUE(e10.is_array(), "e10 is not an array after index assignment");
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect number of elements after assignment");

	e10.insert_at(1, json::Value("new"));
	ASSERT_EQ(e10.size(), 3, "e10 has incorrect size after insert");
	ASSERT_EQ(e10[1].as_string(), "new", "e10[1] has incorrect value after insert");
	ASSERT_EQ(e10[2].as_string(), "foobar", "e10[2] has incorrect value after insert");

	e10.del_at(0);
	ASSERT_EQ(e10.size(), 2, "e10 has incorrect size after delete");
	ASSERT_EQ(e10[1].as_string(), "foobar", "e10[1] has incorrect value after delete");

	e10.clear();
	ASSERT_EQ(e10.size(), 0, "e10 has incorrect number of elements after clear");

	json::Value e11(json::object());
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");

	e11.set_key("foo", json::Value("test"));
	ASSERT_EQ(e11.size(), 1, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["foo"].as_string(), "test", "e11.foo has incorrect value after assignment");

	e11.set_key("foo", json::Value("again"));
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");
	ASSERT_EQ(e11.size(), 1, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["foo"].as_string(), "again", "e11.foo has incorrect value after assignment");

	e11.set_key("bar", json::Value("test"));
	ASSERT_TRUE(e11.is_object(), "e11 is not an object after property assignment");
	ASSERT_EQ(e11.size(), 2, "e11 has incorrect number of properties after assignment");
	ASSERT_EQ(e11["bar"].as_string(), "test", "e11.foo has incorrect value after assignment");

	e11.clear();
	ASSERT_EQ(e11.size(), 0, "e11 has incorrect number of properties after clear");

	json::Value e12(json::object());
	e12.set_key("foo", json::Value("test"));
	e12.set_key("bar", json::Value(3));
	char* out_cstr = e12.save_string(0);
	std::string out(out_cstr);
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

	const json::Value e13(e12);
	ASSERT_EQ(e13["bar"].as_integer(), 3, "e13.bar has incorrect value after copy");

	json::Value e14(json::object());
	ASSERT_TRUE(e14.is_object(), "e14 is not an object after construction");
	e14.set_key("foo", json::object());
	ASSERT_TRUE(e14["foo"].is_object(), "e14.foo is not an object after assignment");
	e14["foo"]["bar"] = json::Value(42);
	ASSERT_EQ(e14["foo"]["bar"].as_integer(), 42, "e14.foo.bar has incorrect value after assignment");

	json::Value e15(json::array());
	ASSERT_TRUE(e15.is_array(), "e15 is not an array after construction");
	e15.set_at(0, json::Value(42));
	ASSERT_EQ(e15[0].as_integer(), 42, "e15[0] has incorrect value after assignment");
	e15[0] = json::Value("foo");
	ASSERT_EQ(e15[0].as_string(), "foo", "e15[0] has incorrecy value after assignment");
	return 0;
}
