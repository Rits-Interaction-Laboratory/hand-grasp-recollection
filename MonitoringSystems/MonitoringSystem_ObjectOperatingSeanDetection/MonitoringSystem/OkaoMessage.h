//#ifndef INCLUDED_OKAOSERVER_MESSAGE_H
#define INCLUDED_OKAOSERVER_MESSAGE_H

#include <vector>
#include <string>
#include <iostream>
#include "zmq.hpp"
#include "msgpack.hpp"


namespace OkaoServer
{
	/**
	* @brief �T�[�o�ւ̃��N�G�X�g���b�Z�[�W
	*/
	class RequestMessage
	{
	public:
		std::string param;				///< �p�����[�^�iJSON�`���j
		std::vector<unsigned char> img;	///< �摜�f�[�^
		MSGPACK_DEFINE( param, img );		///< MessagePack�p��`
	};

	/**
	* @brief �T�[�o����̃��v���C���b�Z�[�W
	*/
	class ReplyMessage
	{
	public:
		std::string okao;		///< OKAO Vision�̏������ʁiJSON�`���j
		MSGPACK_DEFINE( okao );	///< MessagePack�p��`
	};

	/**
	* @brief ���N�G�X�g���b�Z�[�W�̎�M����
	*/
	void recvRequestMessage( zmq::socket_t& socket, RequestMessage* reqMsg )
	{
		zmq::message_t request;
		socket.recv( &request );
		msgpack::unpacked msg;
		msgpack::unpack( &msg, reinterpret_cast< const char* >( request.data() ), request.size() );
		msg.get().convert( reqMsg );
	}

	/**
	* @brief ���v���C���b�Z�[�W�̑��M����
	*/
	void sendReplyMessage( zmq::socket_t& socket, const ReplyMessage& repMsg )
	{
		msgpack::sbuffer sbuf;
		msgpack::pack( sbuf, repMsg );
		zmq::message_t reply( sbuf.size() );
		std::memcpy( reply.data(), sbuf.data(), sbuf.size() );
		socket.send( reply );
	}

	/**
	* @brief ���N�G�X�g���b�Z�[�W�̑��M�����i�N���C�A���g�p�j
	*/
	void sendRequestMessage( zmq::socket_t& socket, const RequestMessage& reqMsg )
	{
		msgpack::sbuffer sbuf;
		msgpack::pack( sbuf, reqMsg );
		zmq::message_t request( sbuf.size() );
		std::memcpy( request.data(), sbuf.data(), sbuf.size() );
		socket.send( request );
	}

	/**
	* @brief ���v���C���b�Z�[�W�̎�M�����i�N���C�A���g�p�j
	*/
	void recvReplyMessage( zmq::socket_t& socket, ReplyMessage* repMsg )
	{
		zmq::message_t reply;
		socket.recv( &reply );
		msgpack::unpacked msg;
		msgpack::unpack( &msg, reinterpret_cast< const char* >( reply.data() ), reply.size() );
		msg.get().convert( repMsg );
	}
}

//#endif
