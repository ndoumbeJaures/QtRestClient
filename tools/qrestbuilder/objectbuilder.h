#ifndef OBJECTBUILDER_H
#define OBJECTBUILDER_H

#include "restbuilder.h"

class ObjectBuilder : public RestBuilder
{
public:
	explicit ObjectBuilder();

private:
	TypeInfo parentType;
	QHash<QString, QString> includes;
	QHash<QString, TypeInfo> members;

	void build() override;

	void generateApiObject(const QString &name);
	void generateApiGadget(const QString &name);
	void readMembers();

	QString setter(const QString &name);

	void writeProperties(bool withNotify);
	void writeReadDeclarations();
	void writeWriteDeclarations();
	void writeNotifyDeclarations();
	void writeMemberDefinitions();
	void writeReadDefinitions(const QString &className);
	void writeWriteDefinitions(const QString &className, bool withNotify);
};

#endif // OBJECTBUILDER_H
