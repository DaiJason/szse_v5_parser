// @Copyright 2017, cao.ning, All Rights Reserved
// @Author:   cao.ning
// @Date:     2017/03/01
// @Brief:    ���V5binaryЭ���е��������ݽṹ����

#ifndef __CN_SZSE_BINARY_MD_FIELD_H__
#define __CN_SZSE_BINARY_MD_FIELD_H__

#include "szse_binary_type.hpp"
#include "szse_binary_field.hpp"

namespace cn
{
namespace szse
{
namespace binary
{

// @Class:   MsgHeader
// @Author:  cao.ning
// @Date:    2017/02/21
// @Brief:
template <is_mutable b>
class MsgHeader : public Field<b>
{
public:
    // 
    static size_t SSize()
    {
        static size_t ssize = TypeInt<uint32_t>::mem_size() * 2;
        return ssize;
    }
    virtual uint32_t Type() const { return 0; }
    virtual uint32_t Size() const { return SSize(); }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, MsgType, BodyLength);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, MsgType, BodyLength);
    }

    TypeInt<uint32_t> MsgType;      // ��Ϣ����
    TypeInt<uint32_t> BodyLength;   // ��Ϣ�峤��
};


// ��¼��Ϣ Logon
template <is_mutable b>
class Logon : public Field<b>
{
public:
    static const uint32_t TypeID = 1;

    TypeCompID          SenderCompID;           // ���ͷ�����
    TypeCompID          TargetCompID;           // ���շ�����
    TypeInt<int32_t>    HeartBtInt;             // �����������λ���롣 �û�����ϵͳ��½ʱ�ṩ���������ء�
    TypeString<16>      Password;               // ����
    TypeString<32>      DefaultApplVerID;       // ������Э��汾����дΪ n.xy ��1.00��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(SenderCompID, TargetCompID, HeartBtInt,
            Password, DefaultApplVerID);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, SenderCompID, TargetCompID,
            HeartBtInt, Password, DefaultApplVerID);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, SenderCompID, TargetCompID,
            HeartBtInt, Password, DefaultApplVerID);
    }
};

// ע����Ϣ Logout
template <is_mutable b>
class Logout : public Field<b>
{
public:
    static const uint32_t TypeID = 2;
    static const int32_t kSessionStatus_type_0 = 0;     //�Ự��Ծ
    static const int32_t kSessionStatus_type_1 = 1;     //�Ự�����Ѹ���
    static const int32_t kSessionStatus_type_2 = 2;     //�����ڵĻỰ����
    static const int32_t kSessionStatus_type_3 = 3;     //�»Ự������Ϲ淶
    static const int32_t kSessionStatus_type_4 = 4;     //�Ự�˵����
    static const int32_t kSessionStatus_type_5 = 5;     //���Ϸ����û��������
    static const int32_t kSessionStatus_type_6 = 6;     //�˻�����
    static const int32_t kSessionStatus_type_7 = 7;     //��ǰʱ�䲻�����¼
    static const int32_t kSessionStatus_type_8 = 8;     //�������
    static const int32_t kSessionStatus_type_9 = 9;     //�յ��� MsgSeqNum(34)̫С
    static const int32_t kSessionStatus_type_10 = 10;    //�յ��� NextExpectedMsgSeqNum(789)̫��.
    static const int32_t kSessionStatus_type_101 = 101;   //����
    static const int32_t kSessionStatus_type_102 = 102;   //��Ч��Ϣ

    TypeInt<int32_t>    SessionStatus;          // �˳�ʱ�ĻỰ״̬
    TypeString<200>     Text;                   // ע��ԭ��Ľ�һ������˵��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(SessionStatus, Text);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, SessionStatus, Text);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, SessionStatus, Text);
    }
};

// ������Ϣ  Heartbeat
template <is_mutable b>
class Heartbeat : public Field<b>
{
public:
    static const uint32_t TypeID = 3;
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override { return 0; }
    virtual bool Load(const char*, size_t) override { return true; }
    virtual bool Write(char*, size_t) override { return true; }
};

// ҵ��ܾ���Ϣ BusinessReject
template <is_mutable b>
class BusinessReject : public Field<b>
{
public:
    static const uint32_t TypeID = 8;
public:
    TypeSeqNum              RefSeqNum;              // ���ܾ���Ϣ����Ϣ���
    TypeInt<uint32_t>       RefMsgType;             // ���ܾ�����Ϣ����
    TypeString<10>          BusinessRejectRefID;    // ���ܾ���Ϣ��Ӧ��ҵ���ID
    TypeInt<uint16_t>       BusinessRejectReason;   // �ܾ�ԭ��
    TypeString<50>          BusinessRejectText;     // �ܾ�ԭ��˵��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(RefSeqNum, RefMsgType, BusinessRejectRefID,
            BusinessRejectReason, BusinessRejectText);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, RefSeqNum, RefMsgType,
            BusinessRejectRefID, BusinessRejectReason, BusinessRejectText);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, RefSeqNum, RefMsgType,
            BusinessRejectRefID, BusinessRejectReason, BusinessRejectText);
    }
};

// Ƶ��������Ϣ  ChannelHeartbeat
template <is_mutable b>
class ChannelHeartbeat : public Field<b>
{
public:
    static const uint32_t TypeID = 390095;
public:
    TypeInt<uint16_t>    ChannelNo;          // Ƶ������
    TypeSeqNum          ApplLastSeqNum;     // ���һ��������Ϣ�ļ�¼��
    TypeBoolean         EndOfChannel;       // Ƶ��������־
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size,
            ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size,
            ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
};

// ������Ϣ
template <is_mutable b>
class Announcement : public Field<b>
{
public:
    static const uint32_t TypeID = 390012;
public:
    TypeLocalTimeStamp      OrigTime;          // ����ʱ��
    TypeInt<uint16_t>       ChannelNo;         // Ƶ������
    TypeString<8>           NewsID;            // ����Ψһ��ʶ
    TypeString<128>         Headline;          // �������
    TypeString<8>           RawDataFormat;     // ���������ݸ�ʽ
    TypeLength              RawDataLength;     // ���������ݳ���
    const char*             RawData;           // ���������ݣ��䳤��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, NewsID, Headline,
            RawDataFormat, RawDataLength)
            + RawDataLength.get_value();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NewsID, Headline, RawDataFormat, RawDataLength))
        {
            RawData = static_cast<const char*>(mem_addr);
            return true;
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NewsID, Headline, RawDataFormat, RawDataLength))
        {
            if (RawDataLength.get_value() >= mem_size)
            {
                memcpy(mem_addr, RawData, RawDataLength.get_value());
                return true;
            }
        }
        return false;
    }
};

// �ش���Ϣ  ReTransmit
template <is_mutable b>
class ReTransmit : public Field<b>
{
public:
    static const uint32_t TypeID = 390094;

    static const uint8_t kResendType_type_1 = 1;      //�����������
    static const uint8_t kResendType_type_2 = 2;      //������Ϣ

    static const uint8_t kResendStatus_type_1 = 1;     //���
    static const uint8_t kResendStatus_type_2 = 2;     //������ɣ��в������������û�з��أ�
    static const uint8_t kResendStatus_type_3 = 3;     //��Ȩ��
    static const uint8_t kResendStatus_type_4 = 4;     //���ݲ�����
public:
    TypeInt<uint8_t>    ResendType;         // �ش�����
    TypeInt<uint16_t>   ChannelNo;          // Ƶ������
    TypeSeqNum          ApplBegSeqNum;      // ��ʼ��ţ��� ResendType=1 ʱ��Ч��ָ����¼��ŵķ�Χ
    TypeSeqNum          ApplEndSeqNum;      // ������ţ��� ResendType=1 ʱ��Ч���� ApplEndSeqNum=0 ʱ��
    // �������ػὫ ApplEndSeqNum ����Ϊ�յ��ش�����ʱ��
    // ��Ƶ���������ڴ��е����ֵ
    TypeString<8>        NewsID;            // ����Ψһ��ʶ���� ResendType=2 ʱ��Ч��Ϊ��ʱ��ʾ���빫���Ҫ
    TypeInt<uint8_t>     ResendStatus;      // �ش�״̬��������������ǰ�û����ظ��û�����ϵͳ������ʱ��Ч
    TypeString<16>       RejectText;        // ������������ǰ�û����ظ��û�����ϵͳ������ʱ��Ч
    // ���������������ǰ�û��ܾ����������������򷵻�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ResendType, ChannelNo, ApplBegSeqNum,
            ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, ResendType, ChannelNo,
            ApplBegSeqNum, ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, ResendType, ChannelNo,
            ApplBegSeqNum, ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
};

// �г�ʵʱ״̬ MarketStatus
template <is_mutable b>
class MarketStatus : public Field<b>
{
public:
    static const uint32_t TypeID = 390019;
public:
    TypeLocalTimeStamp      OrigTime;            // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;           // Ƶ������
    TypeString<8>           MarketID;            // �г�����
    TypeString<8>           MarketSegmentID;     // �г������룬Ԥ��
    TypeString<4>           TradingSessionID;    // ���׻Ự ID
    TypeString<4>           TradingSessionSubID; // ���׻Ự�� ID
    TypeInt<uint16_t>       TradSesStatus;       // ���׻Ự״̬��Ԥ��
    TypeLocalTimeStamp      TradSesStartTime;    // ���׻Ự��ʼʱ�䣬Ԥ��
    TypeLocalTimeStamp      TradSesEndTime;      // ���׻Ự����ʱ�䣬Ԥ��
    TypeAmt                 ThresholdAmount;     // ÿ�ճ�ʼ���
    TypeAmt                 PosAmt;              // ����ʣ���ȣ���Ȳ�����ʱ�������̶�ֵ 0.0000
    TypeString<1>           AmountStatus;        // ���״̬
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, MarketID, MarketSegmentID,
            TradingSessionID, TradingSessionSubID, TradSesStatus,
            TradSesStartTime, TradSesEndTime, ThresholdAmount, PosAmt,
            AmountStatus);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            MarketID, MarketSegmentID, TradingSessionID, TradingSessionSubID,
            TradSesStatus, TradSesStartTime, TradSesEndTime, ThresholdAmount,
            PosAmt, AmountStatus);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            MarketID, MarketSegmentID, TradingSessionID, TradingSessionSubID,
            TradSesStatus, TradSesStartTime, TradSesEndTime, ThresholdAmount,
            PosAmt, AmountStatus);
    }
};

// ֤ȯʵʱ״̬ SecurityStatus
template <is_mutable b>
class SecurityStatus : public Field<b>
{
public:
    static const uint32_t TypeID = 390013;
    static const uint16_t kSecuritySwitchType_type_1 = 1;      // ��������(���������ʱ��֤ȯ)
    static const uint16_t kSecuritySwitchType_type_2 = 2;      // ��ȯ����(��������ȯ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_3 = 3;      // �깺(������ ETF�� LOF �ȿ���ʽ���𣬶��ڻƽ� ETF ��ָ�ֽ��깺)
    static const uint16_t kSecuritySwitchType_type_4 = 4;      // ���(������ ETF�� LOF �ȿ���ʽ���𣬶��ڻƽ� ETF ��ָ�ֽ���ؿ���)
    static const uint16_t kSecuritySwitchType_type_5 = 5;      // �Ϲ�(���������Ϸ����Ϲ�����)
    static const uint16_t kSecuritySwitchType_type_6 = 6;      // ת��(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_7 = 7;      // ����(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_8 = 8;      // ��Ȩ(�����ڴ�����Ȩ�ڵ�Ȩ֤����Ȩ)
    static const uint16_t kSecuritySwitchType_type_10 = 10;      // �򿪲�(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_11 = 11;      // ������(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_12 = 12;      // �ƽ� ETFʵ����(�����ڻƽ� ETF)
    static const uint16_t kSecuritySwitchType_type_13 = 13;      // �ƽ�ETFʵ����(�����ڻƽ� ETF)
    static const uint16_t kSecuritySwitchType_type_14 = 14;      // Ԥ��ҪԼ(�����ڴ���ҪԼ�չ��ڵĹ�Ʊ)
    static const uint16_t kSecuritySwitchType_type_15 = 15;      // ���ҪԼ(�����ڴ���ҪԼ�չ��ڵĹ�Ʊ)
    static const uint16_t kSecuritySwitchType_type_18 = 18;      // ת�ɳ���(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_19 = 19;      // ���۳���(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_20 = 20;      // ��Ѻ(��������Ѻʽ�ع�����Ѻ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_21 = 21;      // ��Ѻ(��������Ѻʽ�ع�����Ѻ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_22 = 22;      // ���Ȩ(���������ȹ�)
    static const uint16_t kSecuritySwitchType_type_23 = 23;      // ��Ʊ��Ѻʽ��(�����ڿɿ�չ��Ʊ��Ѻʽ�ع�ҵ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_24 = 24;      // ʵʱ�ֲ�(�����ڷּ�����)
    static const uint16_t kSecuritySwitchType_type_25 = 25;      // ʵʱ�ϲ�(�����ڷּ�����)
    static const uint16_t kSecuritySwitchType_type_26 = 26;      // ���ҿ���(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_27 = 27;      // �����̱���(��������Ȩ��֧�������̱��۵�֤ȯ)
    static const uint16_t kSecuritySwitchType_type_28 = 28;      // �۹�ͨ������
    static const uint16_t kSecuritySwitchType_type_29 = 29;      // �۹�ͨ������
    static const uint16_t kSecuritySwitchType_type_30 = 30;      // �۹�ͨ�����
    static const uint16_t kSecuritySwitchType_type_31 = 31;      // �۹�ͨ�����
public:
    TypeLocalTimeStamp      OrigTime;          // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;         // Ƶ������
    TypeSecurityID          SecurityID;        // ֤ȯ����
    TypeString<4>           SecurityIDSource;  // ֤ȯ����Դ,102=����֤ȯ������,103=��۽�����
    TypeString<8>           FinancialStatus;   // ֤ȯ״̬
    // A = ���й�˾�����¶��ʾ
    // B = ���й�˾�����¶��ʾ
    // ÿ���ֽڽ�ʾһ��״̬������ͬʱ��ʾ����״̬
    TypeNumInGroup         NoSwitch;            // ���ظ���
    struct SecuritySwitch : public Field<b>
    {
        TypeInt<uint16_t>   SecuritySwitchType;     // �������
        TypeBoolean         SecuritySwitchStatus;   // ����״̬
        virtual uint32_t Size() const override
        {
            return byte_size_sum(SecuritySwitchType, SecuritySwitchStatus);
        }
        virtual bool Load(const char* mem_addr, size_t mem_size) override
        {
            return load_from_memory(mem_addr, mem_size,
                SecuritySwitchType, SecuritySwitchStatus);
        }
        virtual bool Write(char* mem_addr, size_t mem_size) override
        {
            return write_into_memory(mem_addr, mem_size,
                SecuritySwitchType, SecuritySwitchStatus);
        }
    };
    TypeFieldArray<SecuritySwitch>      SecuritySwitchArray; // ��������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, SecurityID, SecurityIDSource,
            FinancialStatus, NoSwitch)
            + SecuritySwitchArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            SecurityID, SecurityIDSource, FinancialStatus, NoSwitch)
            && SecuritySwitchArray.Load(mem_addr, mem_size, NoSwitch.get_value());
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            SecurityID, SecurityIDSource, FinancialStatus, NoSwitch)
            && SecuritySwitchArray.Write(mem_addr, mem_size);
    }
};

// ��������Ƶ��ͳ��
template <is_mutable b>
class MarketSnapshotStatistic : public Field<b>
{
public:
    static const uint32_t TypeID = 390090;
    TypeLocalTimeStamp      OrigTime;           // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeNumInGroup          NoMDStreamID;       // ����������
    struct StreamStatistic : public Field<b>
    {
        TypeString<3>           MDStreamID;         // �������
        TypeInt<uint32_t>       StockNum;           // ֤ȯֻ��
        TypeString<8>           TradingPhaseCode;   // ����״̬���� 0 λ��T=�������ۣ�E=�ѱ���
        virtual uint32_t Size() const override
        {
            return byte_size_sum(MDStreamID, StockNum, TradingPhaseCode);
        }
        virtual bool Load(const char* mem_addr, size_t mem_size) override
        {
            return load_from_memory(mem_addr, mem_size, MDStreamID, StockNum,
                TradingPhaseCode);
        }
        virtual bool Write(char* mem_addr, size_t mem_size) override
        {
            return write_into_memory(mem_addr, mem_size, MDStreamID, StockNum,
                TradingPhaseCode);
        }
    };
    TypeFieldArray<StreamStatistic>    StatisticArray; // ͳ������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, NoMDStreamID)
            + StatisticArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NoMDStreamID)
            && StatisticArray.Load(mem_addr, mem_size, NoMDStreamID.get_value());
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NoMDStreamID)
            && StatisticArray.Write(mem_addr, mem_size);
    }
};

// ��������Ĺ����ֶ�
template <is_mutable b>
class MarketSnapshotBase : public Field<b>
{
public:
    TypeLocalTimeStamp      OrigTime;           // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeString<3>           MDStreamID;         // �������
    // 010 �ֻ�����Ʊ������ծȯ�ȣ����о��۽��׿�������
    // 020 ��Ѻʽ�ع����׿�������
    // 030 ծȯ������������
    // 040 ��Ȩ���о��۽��׿�������
    // 060 �����̼۽��׵��̺󶨼۽��׿�������
    // 061 �Գɽ�����Ȩƽ���۽��׵��̺󶨼۽��׿�������
    // 900 ָ����������
    // 910 �ɽ���ͳ��ָ���������
    TypeSecurityID          SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypeString<8>           TradingPhaseCode;   // ��Ʒ�����Ľ��׽׶δ���
    // �� 0 λ��
    // S=����������ǰ����O=���̼��Ͼ��ۣ�T=�������ۣ�B=����
    // C=���̼��Ͼ��ۣ�E=�ѱ��У�H=��ʱͣ�ƣ�A=�̺���
    // V=�������ж�
    // �� 1 λ��
    // 0=����״̬��1=ȫ��ͣ��
    TypePrice               PrevClosePx;        // ���ռ�
    TypeInt<int64_t>        NumTrades;          // �ɽ�����
    TypeQty                 TotalVolumeTrade;   // �ɽ�����
    TypeAmt                 TotalValueTrade;    // �ɽ��ܽ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, MDStreamID, SecurityID,
            SecurityIDSource, TradingPhaseCode, PrevClosePx,
            NumTrades, TotalVolumeTrade, TotalValueTrade);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return load_from_memory(mem_tail_, mem_size, OrigTime,
            ChannelNo, MDStreamID, SecurityID, SecurityIDSource,
            TradingPhaseCode, PrevClosePx, NumTrades, TotalVolumeTrade,
            TotalValueTrade);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return write_into_memory((char*&)mem_tail_, mem_size, OrigTime, ChannelNo,
            MDStreamID, SecurityID, SecurityIDSource, TradingPhaseCode,
            PrevClosePx, NumTrades, TotalVolumeTrade, TotalValueTrade);
    }
protected:
    // ��¼�ײ���β��ָ�룬��Load��Writeʱ���£�Ϊ��������ִ�к�����Load��Write����
    // ���ڴ�֮ǰִ����Load����head��tailָ���ȡ�ڴ���ײ����ڴ��β��
    // ���ڴ�֮ǰִ����Write����ָ�����д���ڴ���ײ���β��
    const char* mem_head_;
    const char* mem_tail_;
};

// ���о��۽���ҵ��������� MarketSnapshot_300111
// 010 �ֻ�����Ʊ������ծȯ�ȣ����о��۽��׿�������
// 020 ��Ѻʽ�ع����׿�������
// 030 ծȯ������������
// 040 ��Ȩ���о��۽��׿�������
template <is_mutable b>
class MarketSnapshot_300111 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300111;
public:
    // Extend Fields ҵ����չ�ֶ�
    TypeNumInGroup          NoMDEntries;            // ������Ŀ����
    struct SecurityEntry : public Field<b>
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���
        TypeInt<int64_t>        MDEntryPx;       // �۸񣬳���1000000
        TypeQty                 MDEntrySize;     // ����
        TypeInt<uint16_t>       MDPriceLevel;    // �����̵�λ
        TypeInt<int64_t>        NumberOfOrders;  // ��λ��ί�б�����Ϊ 0 ��ʾ����ʾ
        TypeNumInGroup          NoOrders;        // ��λ��ʾί�б�����Ϊ 0 ��ʾ����ʾ
        struct OrderQty : public Field<b>
        {
            TypeQty             Qty;
            virtual uint32_t Size() const override { return TypeQty::mem_size(); }
            virtual bool Load(const char* mem_addr, size_t mem_size) override
            {
                return load_from_memory(mem_addr, mem_size, Qty);
            }
            virtual bool Write(char* mem_addr, size_t mem_size) override
            {
                return write_into_memory(mem_addr, mem_size, Qty);
            }
        };
        TypeFieldArray<OrderQty>       OrderQtyArray;        // ί������
        uint32_t Size() const override
        {
            return byte_size_sum(MDEntryType, MDEntryPx, MDEntrySize,
                MDPriceLevel, NumberOfOrders, NoOrders)
                + OrderQtyArray.Size();
        }
        bool Load(const char* mem_addr, size_t mem_size) override
        {
            return load_from_memory(mem_addr, mem_size, MDEntryType,
                MDEntryPx, MDEntrySize, MDPriceLevel, NumberOfOrders,
                NoOrders)
                && OrderQtyArray.Load(mem_addr, mem_size, NoOrders.get_value());
        }
        bool Write(char* mem_addr, size_t mem_size) override
        {
            return write_into_memory(mem_addr, mem_size, MDEntryType,
                MDEntryPx, MDEntrySize, MDPriceLevel, NumberOfOrders,
                NoOrders) && OrderQtyArray.Write(mem_addr, mem_size);
        }
    };
    TypeFieldArray<SecurityEntry> SecurityEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + SecurityEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};


// �̺󶨼۽���ҵ��������� MarketSnapshot_300611
// 060 �����̼۽��׵��̺󶨼۽��׿�������
// 061 �Գɽ�����Ȩƽ���۽��׵��̺󶨼۽��׿�������
template <is_mutable b>
class MarketSnapshot_300611 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300611;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeNumInGroup   NoMDEntries;            // ������Ŀ����
    struct SecurityEntry : public Field<b>
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���0=���룻1=����
        TypeInt<int64_t>        MDEntryPx;       // �۸�
        TypeQty                 MDEntrySize;     // ����
        virtual uint32_t Size() const override
        {
            return byte_size_sum(MDEntryType, MDEntryPx, MDEntrySize);
        }
        virtual bool Load(const char* mem_addr, size_t mem_size) override
        {
            return load_from_memory(mem_addr, mem_size,
                MDEntryType, MDEntryPx, MDEntrySize);
        }
        virtual bool Write(char* mem_addr, size_t mem_size) override
        {
            return write_into_memory(mem_addr, mem_size,
                MDEntryType, MDEntryPx, MDEntrySize);
        }
    };
    TypeFieldArray<SecurityEntry> SecurityEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + SecurityEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};

// ָ��������� MarketSnapshot_309011
// 900 ָ����������
template <is_mutable b>
class MarketSnapshot_309011 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 309011;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeNumInGroup   NoMDEntries;            // ������Ŀ����
    struct IndexEntry : public Field<b>
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���
        TypeInt<int64_t>        MDEntryPx;       // ָ����λ
        virtual uint32_t Size() const override
        {
            return byte_size_sum(MDEntryType, MDEntryPx);
        }
        virtual bool Load(const char* mem_addr, size_t mem_size) override
        {
            return load_from_memory(mem_addr, mem_size, MDEntryType, MDEntryPx);
        }
        virtual bool Write(char* mem_addr, size_t mem_size) override
        {
            return write_into_memory(mem_addr, mem_size, MDEntryType, MDEntryPx);
        }
    };
    TypeFieldArray<IndexEntry> IndexEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + IndexEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && IndexEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && IndexEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};

// �ɽ���ͳ��ָ��������� MarketSnapshot_309111
// 910 �ɽ���ͳ��ָ���������
template <is_mutable b>
class MarketSnapshot_309111 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 309111;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeInt<uint32_t>   StockNum;               // ͳ����ָ����������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + StockNum.mem_size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, StockNum);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, StockNum);
        }
        return false;
    }
};

// ���ί������Ĺ����ֶ�
template <is_mutable b>
class OrderSnapshotBase : public Field<b>
{
public:
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeSeqNum              ApplSeqNum;         // ��Ϣ��¼�ţ��� 1 ��ʼ����
    TypeString<3>           MDStreamID;         // �������
    // 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
    // 021 ��Ѻʽ�ع������������
    // 041 ��Ȩ���о��۽����������
    // 051 Э�齻�������������
    // 052 Э�齻����ʶ�������
    // 071 ת��֤ͨȯ�����������
    TypeSecurityID       SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypePrice            Price;              // ί�м۸�
    TypeQty              OrderQty;           // ί������
    TypeString<1>           Side;               // ��������1=��2 = ����G = ���룻F = ����
    TypeLocalTimeStamp   OrderTime;          // ί��ʱ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplSeqNum, MDStreamID, SecurityID,
            SecurityIDSource, Price, OrderQty, Side, OrderTime);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return load_from_memory(mem_tail_, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, SecurityID, SecurityIDSource, Price,
            OrderQty, Side, OrderTime);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return write_into_memory((char*&)mem_tail_, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, SecurityID, SecurityIDSource, Price,
            OrderQty, Side, OrderTime);
    }
protected:
    const char* mem_head_;
    const char* mem_tail_;
};

// ���о���ҵ�����ί������ OrderSnapshot_300192
// 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
// 021 ��Ѻʽ�ع������������
// 041 ��Ȩ���о��۽����������
template <is_mutable b>
class OrderSnapshot_300192 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300192;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeString<2>   OrdType;               // �������1 = �мۣ�2 = �޼ۣ�U = ��������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + OrdType.mem_size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, OrdType);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, OrdType);
        }
        return false;
    }
};

// Э�齻��ҵ�����ί������ OrderSnapshot_300592
// 051 Э�齻�������������
// 052 Э�齻����ʶ�������
template <is_mutable b>
class OrderSnapshot_300592 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300592;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeString<8>   ConfirmID;          // ��������Լ���ţ�ConfirmID Ϊ�ձ�ʾ���������飬����Ϊ��������
    TypeString<12>  Contactor;          // ��ϵ��
    TypeString<30>  ContactInfo;        // ��ϵ��ʽ
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + byte_size_sum(ConfirmID, Contactor, ContactInfo);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size,
                ConfirmID, Contactor, ContactInfo);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, ConfirmID, Contactor, ContactInfo);
        }
        return false;
    }
};

// ת��֤ͨȯ����ҵ�����ί������ OrderSnapshot_300792
// 071 ת��֤ͨȯ�����������
template <is_mutable b>
class OrderSnapshot_300792 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300792;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeInt<uint16_t>   ExpirationDays;               // ���ޣ���λΪ����
    TypeInt<uint8_t>    ExpirationType;               // �������ͣ�1 = �̶�����
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + byte_size_sum(ExpirationDays, ExpirationType);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, ExpirationDays, ExpirationType);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, ExpirationDays, ExpirationType);
        }
        return false;
    }
};

// ��ʳɽ�����Ĺ����ֶ�
template <is_mutable b>
class TransactionSnapshotBase : public Field<b>
{
public:
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeSeqNum           ApplSeqNum;         // ��Ϣ��¼�ţ��� 1 ��ʼ����
    TypeString<3>           MDStreamID;         // �������
    // 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
    // 021 ��Ѻʽ�ع������������
    // 041 ��Ȩ���о��۽����������
    // 051 Э�齻�������������
    // 052 Э�齻����ʶ�������
    // 071 ת��֤ͨȯ�����������
    TypeSeqNum           BidApplSeqNum;      // ��ί���������� 1 ��ʼ������ 0 ��ʾ�޶�Ӧί��
    TypeSeqNum           OfferApplSeqNum;    // ����ί���������� 1 ��ʼ������ 0 ��ʾ�޶�Ӧί��
    TypeSecurityID       SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypePrice            LastPx;             // ί�м۸�
    TypeQty              LastQty;            // ί������
    TypeString<1>           ExecType;           // �ɽ����4 = ������F = �ɽ�
    TypeLocalTimeStamp   TransactTime;       // ί��ʱ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplSeqNum, MDStreamID, BidApplSeqNum,
            OfferApplSeqNum, SecurityID, SecurityIDSource, LastPx, LastQty,
            ExecType, TransactTime);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, BidApplSeqNum, OfferApplSeqNum, SecurityID,
            SecurityIDSource, LastPx, LastQty, ExecType, TransactTime);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, BidApplSeqNum, OfferApplSeqNum, SecurityID,
            SecurityIDSource, LastPx, LastQty, ExecType, TransactTime);
    }
};

// ���о���ҵ����ʳɽ����� TransactionSnapshot_300191
// 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
// 021 ��Ѻʽ�ع������������
// 041 ��Ȩ���о��۽����������
template <is_mutable b>
class TransactionSnapshot_300191 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300191;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

// Э�齻��ҵ����ʳɽ����� TransactionSnapshot_300591
// 051 Э�齻�������������
// 052 Э�齻����ʶ�������
template <is_mutable b>
class TransactionSnapshot_300591 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300591;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

// ת��֤ͨȯ����ҵ����ʳɽ����� TransactionSnapshot_300791
// 071 ת��֤ͨȯ�����������
template <is_mutable b>
class TransactionSnapshot_300791 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300791;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

namespace immutable_
{
typedef cn::szse::binary::MsgHeader<false> MsgHeader;
typedef cn::szse::binary::Logon<false> Logon;
typedef cn::szse::binary::Logout<false> Logout;
typedef cn::szse::binary::Heartbeat<false> Heartbeat;
typedef cn::szse::binary::BusinessReject<false> BusinessReject;
typedef cn::szse::binary::ChannelHeartbeat<false> ChannelHeartbeat;
typedef cn::szse::binary::Announcement<false> Announcement;
typedef cn::szse::binary::ReTransmit<false> ReTransmit;
typedef cn::szse::binary::MarketStatus<false> MarketStatus;
typedef cn::szse::binary::SecurityStatus<false> SecurityStatus;
typedef cn::szse::binary::MarketSnapshotStatistic<false> MarketSnapshotStatistic;
typedef cn::szse::binary::MarketSnapshotBase<false> MarketSnapshotBase;
typedef cn::szse::binary::MarketSnapshot_300111<false> MarketSnapshot_300111;
typedef cn::szse::binary::MarketSnapshot_300611<false> MarketSnapshot_300611;
typedef cn::szse::binary::MarketSnapshot_309011<false> MarketSnapshot_309011;
typedef cn::szse::binary::MarketSnapshot_309111<false> MarketSnapshot_309111;
typedef cn::szse::binary::OrderSnapshotBase<false> OrderSnapshotBase;
typedef cn::szse::binary::OrderSnapshot_300192<false> OrderSnapshot_300192;
typedef cn::szse::binary::OrderSnapshot_300592<false> OrderSnapshot_300592;
typedef cn::szse::binary::OrderSnapshot_300792<false> OrderSnapshot_300792;
typedef cn::szse::binary::TransactionSnapshotBase<false> TransactionSnapshotBase;
typedef cn::szse::binary::TransactionSnapshot_300191<false> TransactionSnapshot_300191;
typedef cn::szse::binary::TransactionSnapshot_300591<false> TransactionSnapshot_300591;
typedef cn::szse::binary::TransactionSnapshot_300791<false> TransactionSnapshot_300791;
} // namespace immutable_ END

namespace mutable_
{
typedef cn::szse::binary::MsgHeader<true> MsgHeader;
typedef cn::szse::binary::Logon<true> Logon;
typedef cn::szse::binary::Logout<true> Logout;
typedef cn::szse::binary::Heartbeat<true> Heartbeat;
typedef cn::szse::binary::BusinessReject<true> BusinessReject;
typedef cn::szse::binary::ChannelHeartbeat<true> ChannelHeartbeat;
typedef cn::szse::binary::Announcement<true> Announcement;
typedef cn::szse::binary::ReTransmit<true> ReTransmit;
typedef cn::szse::binary::MarketStatus<true> MarketStatus;
typedef cn::szse::binary::SecurityStatus<true> SecurityStatus;
typedef cn::szse::binary::MarketSnapshotStatistic<true> MarketSnapshotStatistic;
typedef cn::szse::binary::MarketSnapshotBase<true> MarketSnapshotBase;
typedef cn::szse::binary::MarketSnapshot_300111<true> MarketSnapshot_300111;
typedef cn::szse::binary::MarketSnapshot_300611<true> MarketSnapshot_300611;
typedef cn::szse::binary::MarketSnapshot_309011<true> MarketSnapshot_309011;
typedef cn::szse::binary::MarketSnapshot_309111<true> MarketSnapshot_309111;
typedef cn::szse::binary::OrderSnapshotBase<true> OrderSnapshotBase;
typedef cn::szse::binary::OrderSnapshot_300192<true> OrderSnapshot_300192;
typedef cn::szse::binary::OrderSnapshot_300592<true> OrderSnapshot_300592;
typedef cn::szse::binary::OrderSnapshot_300792<true> OrderSnapshot_300792;
typedef cn::szse::binary::TransactionSnapshotBase<true> TransactionSnapshotBase;
typedef cn::szse::binary::TransactionSnapshot_300191<true> TransactionSnapshot_300191;
typedef cn::szse::binary::TransactionSnapshot_300591<true> TransactionSnapshot_300591;
typedef cn::szse::binary::TransactionSnapshot_300791<true> TransactionSnapshot_300791;
} // namespace mutable_ END


} // namespace binary END
} // namespace szse END
} // namespace cn END

#endif // __CN_SZSE_BINARY_MD_FIELD_H__