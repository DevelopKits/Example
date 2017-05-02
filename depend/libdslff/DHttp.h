#ifndef INCLUDED_LIBDSL_DHTTP_H
#define INCLUDED_LIBDSL_DHTTP_H

#include <libdsl/dslbase.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DHttp
{
public:
	DHttp();
	~DHttp();
	/**
	 * �����л����� ���DHttp�ṹ�� 
	 * ͬһ��Pdu���Էֶ�����룬���Ǳ��뱣֤��һ���������HTTP��ͷ��������
	 * @param data �������ݵ��׵�ַ
	 * @param len �������ݵĳ��ȣ����Ϊ������ʾ���������Ϊ�ַ����������ڲ����Զ������ַ�����β
	 * @return ������bodyͷ������ ��������������� 0��body���岻���� ���������ȫ������ ������������� �������ĵ����ݳ���
	 * ������0ʱ �Ϳ��Զ�ȡHTTPͷ����Ĳ���
	 */
	int fromStream(const char * data, int len = -1);

	/**
	 * �ж��Ƿ�Ϊ�����
	 * @return true:����� false:�ظ���
	 */
	bool IsRequest() const;

	/**
	 * ��ȡЭ��ķ���
	 * @return Э������ �� GET POST ...
	 */
	const char * GetMethod() const;

	/**
	 * ��ȡURL
	 * @return URL�ֶ�
	 */
	const char * GetUrl() const;

	/**
	 * ��ȡ״̬��
	 * @return ״̬��
	 */
	int GetStatus() const;

	/**
	 * ��ȡ״̬�������ֶ�
	 * @return ״̬�������ֶ�
	 */
	const char * GetReason() const;

	/**
	 * ��ȡ�汾��
	 * @return �汾��
	 */
	const char * GetVersion() const;

	/**
	 * ��ȡͷ����Ϣ
	 * @param headKey ͷ���key
	 * @return ͷ���value
	 */
	const char * GetHeadParam(const char * headKey) const; // ��ȡͷ����Ϣ

	/**
	 * ��ȡHTTP����
	 * @return HTTP������׵�ַ
	 */
	const char * GetBody() const; // ��ȡHTTP����

	/**
	 * ��ȡHTTP���峤��
	 * @return HTTP���峤��
	 */
	int GetBodyLen() const;	// ��ȡHTTP���峤��


	/**
	 * �������������������
	 * @param requestLine �������� ���� method url version ���� POST / HTTP/1.1
	 * @return 0���ɹ�  ������ʧ��
	 */
	int SetRequestLine(const char * requestLine);


	/**
	 * ���ûظ�������������
	 * @param responseLine �������� ���� version status reason ���� HTTP/1.1 200 OK
	 * @return 0���ɹ�  ������ʧ��
	 */
	int SetResponseLine(const char * responseLine);

	/**
	 * ����ͷ������
	 * @param headKey ͷ���key
	 * @param headValue ͷ���value
	 * @return 0���ɹ�  ������ʧ��
	 */
	int SetHeadParam(const char * headKey, const char * headValue);

	/**
	 * ����HTTP��������
	 * @param data ���ݵ�Դ��ַ
	 * @param len ���ݵĳ��ȣ����Ϊ������ʾ���������Ϊ�ַ����������ڲ����Զ������ַ�����β
	 * @return 0���ɹ�  ������ʧ��
	 */
	int SetBody(const char * data, int len = -1);

	/**
	 * ���л�����
	 * @param len ����������������л�������ݳ���
	 * @return 0���ɹ�  ������ʧ��
	 */
	const char* toStream(int &len);


	/**
	 * ��λ����
	 */
	void Reset();

protected:
	class DHttpImpl;
	class DHttpImpl * m_impl;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DHTTP_H

