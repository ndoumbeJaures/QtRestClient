#include "restreplyawaitable.h"
#include "restreplyawaitable_p.h"

#include <QtCore/QMetaEnum>
using namespace QtRestClient;

RestReplyAwaitable::RestReplyAwaitable(RestReply *reply) :
	d{new RestReplyAwaitablePrivate{reply}}
{}

RestReplyAwaitable::RestReplyAwaitable(RestReplyAwaitable &&other) noexcept
{
	d.swap(other.d);
}

RestReplyAwaitable &RestReplyAwaitable::operator=(RestReplyAwaitable &&other) noexcept
{
	d.swap(other.d);
	return *this;
}

RestReplyAwaitable::~RestReplyAwaitable() = default;

void RestReplyAwaitable::prepare(std::function<void ()> resume)
{
	QObject::connect(d->reply, &RestReply::succeeded, d->reply, [this, resume](int, QJsonValue value) {
		d->errorResult.reset();
		d->successResult = std::move(value);
		resume();
	});
	QObject::connect(d->reply, &RestReply::failed, d->reply, [this, resume](int code, const QJsonValue &value) {
		d->errorResult.reset(new AwaitedException{code, RestReply::FailureError, QVariant::fromValue(value)});
		resume();
	});
	QObject::connect(d->reply, &RestReply::error, d->reply, [this, resume](const QString &errorString, int code, RestReply::ErrorType type) {
		d->errorResult.reset(new AwaitedException{code, type, errorString});
		resume();
	});
}

RestReplyAwaitable::type RestReplyAwaitable::result()
{
	if(d->errorResult) {
		d->errorResult->raise();
		Q_UNREACHABLE();
	} else
		return d->successResult;
}



AwaitedException::AwaitedException(int code, RestReply::ErrorType type, QVariant data) :
	_code{code},
	_type{type},
	_data{std::move(data)}
{}

AwaitedException::AwaitedException(const AwaitedException * const other) :
	_code{other->_code},
	_type{other->_type},
	_data{other->_data},
	_msg{other->_msg}
{}

int AwaitedException::errorCode() const
{
	return _code;
}

RestReply::ErrorType AwaitedException::errorType() const
{
	return _type;
}

QVariant AwaitedException::errorData() const
{
	return _data;
}

QJsonObject AwaitedException::errorObject() const
{
	return _data.toJsonObject();
}

QJsonArray AwaitedException::errorArray() const
{
	return _data.toJsonArray();
}

QString AwaitedException::errorString() const
{
	return _data.toString();
}

QString AwaitedException::errorString(const std::function<QString (QJsonObject, int)> &failureTransformer) const
{
	if(_type == RestReply::FailureError)
		return failureTransformer(errorObject(), _code);
	else
		return errorString();
}

QString AwaitedException::errorString(const std::function<QString (QJsonArray, int)> &failureTransformer) const
{
	if(_type == RestReply::FailureError)
		return failureTransformer(errorArray(), _code);
	else
		return errorString();
}

const char *AwaitedException::what() const noexcept
{
	if(_msg.isEmpty()) {
		auto mEnum = QMetaEnum::fromType<RestReply::ErrorType>();
		_msg = QByteArray(mEnum.valueToKey(_type)) + ": " +
			   QByteArray::number(_code) + ", " +
			   _data.toString().toUtf8();
	}
	return _msg.constData();
}

void AwaitedException::raise() const
{
	throw *this;
}

AwaitedException::Base *AwaitedException::clone() const
{
	return new AwaitedException{this};
}

// ------------- PRIVATE IMPLEMENTATION -------------

RestReplyAwaitablePrivate::RestReplyAwaitablePrivate(RestReply *reply) :
	reply{reply}
{}
