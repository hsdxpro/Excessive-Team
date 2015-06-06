#pragma once
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Font.hpp"

#include <assert.h>
#include <string>

#ifdef DEBUG_BUILD
	#define PROFILE_ENGINE
#endif

#ifdef PROFILE_ENGINE
	#define CONCAT_MACRO_BASE(x, y)		x##y
	#define CONCAT_MACRO(x,y)			CONCAT_MACRO_BASE(x,y)

	#define PROFILE_SCOPE(strName) \
		EngineCpuProfiler::Scope CONCAT_MACRO(section_, __COUNTER__)(strName)

#define PROFILE_SCOPE_SUM_HELPER_CONCAT(strName, count) \
	static EngineCpuProfiler::ScopeSum CONCAT_MACRO(_node, count)(strName); \
	EngineCpuProfiler::ScopeSum::LifeCycleHelper CONCAT_MACRO(_helper, count)(&CONCAT_MACRO(_node, count));

	#define PROFILE_SCOPE_SUM(strName) \
		PROFILE_SCOPE_SUM_HELPER_CONCAT(strName, __COUNTER__)
#else
	#define PROFILE_SCOPE(x)	 (x)
	#define PROFILE_SCOPE_SUM(x) (x)
#endif

class ITimer;

class EngineCpuProfiler
{
public:
	struct ProfilerNode
	{
		ProfilerNode():name("INVALID"), profiledSeconds(0), parent(0){}

		std::string name;

		double profiledSeconds;
		std::vector<ProfilerNode*> childs;
		ProfilerNode* parent;
	};

	class Scope
	{
	public:
		Scope(const std::string& name);
		~Scope();

	protected:
		ITimer* timer;
		std::string name;
	};

	class ScopeSum
	{
	public:
		class LifeCycleHelper
		{
		public:
			LifeCycleHelper(ScopeSum* scopeSumProfiler);
			~LifeCycleHelper();
		protected:
			ScopeSum* scopeSumProfiler;
		};

		friend class LifeCycleHelper;
		ScopeSum(const std::string& name);
		~ScopeSum();

	protected:
		std::string name;
		ITimer* timer;
		ProfilerNode* profilerNode;
		size_t ID;
	};

protected:
	EngineCpuProfiler();
	void _internalupdateAndPresent();
	void _internalDrawSectionTreeRecursively(ProfilerNode* node, size_t& curNodePosY_inout, size_t depth);

public:
	static void init();
	static void updateAndPresent();
	static EngineCpuProfiler* GetSingletonInstance();

protected:
	static EngineCpuProfiler* instance;
	static EngineCpuProfiler::ProfilerNode* lastConstructedTreeNode;
	static size_t IDGenerator;

	sf::RenderWindow window;
	sf::Font fontArial;

public:
	// Profiler node tree
	std::vector<ProfilerNode*> treeRootComponents;

	ProfilerNode* lowestPerfSectionNode;
};