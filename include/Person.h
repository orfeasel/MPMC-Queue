#pragma once
#include <string>
#include <iostream>

class Person
{

public:

	Person() : m_name{ "N/A" }, m_age{ -1 }, m_jobTitle{ "N/A" } {}

	Person(std::string name, int age, std::string job) : 
		m_name{name}, m_age(age), m_jobTitle{job}
	{ }

	//Copy constructor
	//No dynamic mem, member wise copy is fine at this point?
	Person(const Person& p) = default;

	//Copy operator assignment
	Person& operator=(const Person& p)
	{
		if (this != &p)
		{
			m_name = p.m_name;
			m_age = p.m_age;
			m_jobTitle = p.m_jobTitle;
		}
		
		return *this;
	}

	//Move con
	Person(Person&& p) noexcept : 
		m_name{ std::move(p.m_name) }, m_age{ p.m_age }, m_jobTitle{std::move(p.m_jobTitle)}
	{
		std::cout << "Move con";
	}

	//Move operator assignment
	Person& operator=(Person&& p) noexcept
	{
		if (this != &p)
		{
			m_name = std::move(p.m_name);
			m_age = std::move(p.m_age);
			m_jobTitle = std::move(p.m_jobTitle);
		}
		
		return *this;
	}

	~Person() 
	{
		std::cout << "Deallocating..." << m_name << '\n';
	}

	friend std::ostream& operator<<(std::ostream& out, const Person& p)
	{
		out << "Person{" << p.m_name << ',' << p.m_age << ',' << p.m_jobTitle << "}\n";
		return out;
	}

private:

	std::string m_name;
	int m_age;
	std::string m_jobTitle;
};

