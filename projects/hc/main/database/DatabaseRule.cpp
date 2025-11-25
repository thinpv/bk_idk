#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"
#include "Rule.h"

#define TABLE_NAME "[Rule]"

// TODO: insert first run

static int RuleParse(sqlite3_stmt *stmt, void *ptr)
{
	// int s, index;
	// if (stmt)
	// {
	// 	while (1)
	// 	{
	// 		s = sqlite3_step(stmt);
	// 		if (s == SQLITE_ROW)
	// 		{
	// 			index = 0;
	// 			string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			string type = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			bool enable = sqlite3_column_int(stmt, index++) ? true : false;
	// 			uint32_t count = sqlite3_column_int(stmt, index++);
	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint64_t updatedAt = sqlite3_column_int64(stmt, index++);
	// 			LOGD("%s, %s, %s, %s", id.c_str(), data.c_str(), type.c_str(), enable ? "true" : "flase");

	// 			string ruledata;
	// 			string decode = macaron::Base64::Decode(data, ruledata);
	// 			if (decode == "")
	// 			{
	// 				Json::Value ruleValue;
	// 				if (ruleValue.parse(ruledata) && ruleValue.isObject())
	// 				{
	// 					Rule *rule = RuleManager::GetInstance()->AddRule(id, ruleValue, false);
	// 					if (rule)
	// 					{
	// 						rule->setEnable(enable);
	// 						// rule->SetFirstRun(isFirstRun);
	// 						rule->Check();
	// 					}
	// 				}
	// 				else
	// 				{
	// 					LOGW("RuleRead json format error rule: %s", ruledata.c_str());
	// 				}
	// 			}
	// 			else
	// 			{
	// 				LOGW("Decode data err: %s", decode.c_str());
	// 			}
	// 		}
	// 		else if (s == SQLITE_DONE)
	// 		{
	// 			return CODE_OK;
	// 		}
	// 		else
	// 		{
	// 			LOGE("RuleParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::RuleRead()
{
	return ReadAll(TABLE_NAME, NULL, RuleParse);
}

int Database::RuleAdd(Rule *rule, string data, int type)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, name, type, enable, count, data, updated_at) VALUES ('" +
							 rule->getId() + "','" +								 // id
							 rule->getName() + "'," +								 // name
							 to_string(type) + ", " +								 // type
							 to_string(rule->getEnable()) + "," +		 // enable
							 to_string(rule->getCount()) + ",'" +		 // count
							 macaron::Base64::Encode(data) + "'," +	 // data
							 to_string(rule->getUpdatedAt()) + ");"; // updated_at
	return Sqlite_Exec(sql);
}

int Database::RuleUpdateData(Rule *rule, string data)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(data) + "' WHERE id='" + rule->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleUpdateStatus(Rule *rule)
{
	string sql = "UPDATE " TABLE_NAME " SET enable=" + to_string(rule->getEnable()) + " WHERE id='" + rule->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleUpdateType(Rule *rule, int type)
{
	string sql = "UPDATE " TABLE_NAME " SET type=" + to_string(type) + " WHERE id='" + rule->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleUpdateFirstRun(Rule *rule, bool isFirstRun)
{
	string sql = "UPDATE " TABLE_NAME " SET is_first_run =" + to_string(isFirstRun) + " WHERE id='" + rule->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleDel(Rule *rule)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id='" + rule->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id='" + id + "';";
	return Sqlite_Exec(sql);
}

int Database::RuleDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
