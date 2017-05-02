#ifndef INCLUDED_LIBDSL_DHTTPPROTOCOL_H
#define INCLUDED_LIBDSL_DHTTPPROTOCOL_H

/* ----------------------- HTTP Status Codes  ------------------------- */

#define DHTTP_CONTINUE                      100
#define DHTTP_SWITCHING_PROTOCOLS           101
#define DHTTP_OK                            200
#define DHTTP_CREATED                       201
#define DHTTP_ACCEPTED                      202
#define DHTTP_NON_AUTHORITATIVE             203
#define DHTTP_NO_CONTENT                    204
#define DHTTP_RESET_CONTENT                 205
#define DHTTP_PARTIAL_CONTENT               206
#define DHTTP_MULTIPLE_CHOICES              300
#define DHTTP_MOVED_PERMANENTLY             301
#define DHTTP_MOVED_TEMPORARILY             302
#define DHTTP_SEE_OTHER                     303
#define DHTTP_NOT_MODIFIED                  304
#define DHTTP_USE_PROXY                     305
#define DHTTP_BAD_REQUEST                   400
#define DHTTP_UNAUTHORIZED                  401
#define DHTTP_PAYMENT_REQUIRED              402
#define DHTTP_FORBIDDEN                     403
#define DHTTP_NOT_FOUND                     404
#define DHTTP_METHOD_NOT_ALLOWED            405
#define DHTTP_NOT_ACCEPTABLE                406
#define DHTTP_PROXY_AUTHENTICATION_REQUIRED 407
#define DHTTP_REQUEST_TIME_OUT              408
#define DHTTP_CONFLICT                      409
#define DHTTP_GONE                          410
#define DHTTP_LENGTH_REQUIRED               411
#define DHTTP_PRECONDITION_FAILED           412
#define DHTTP_REQUEST_ENTITY_TOO_LARGE      413
#define DHTTP_REQUEST_URI_TOO_LARGE         414
#define DHTTP_UNSUPPORTED_MEDIA_TYPE        415
#define DHTTP_INTERNAL_SERVER_ERROR         500
#define DHTTP_NOT_IMPLEMENTED               501
#define DHTTP_BAD_GATEWAY                   502
#define DHTTP_SERVICE_UNAVAILABLE           503
#define DHTTP_GATEWAY_TIME_OUT              504
#define DHTTP_VERSION_NOT_SUPPORTED         505
#define DHTTP_VARIANT_ALSO_VARIES           506




//HTTP Sector
#define METHOD_POST			 "POST"
#define METHOD_GET			 "GET"
#define METHOD_PUT			 "PUT"
#define METHOD_INVALID		 "INVALID"
#define METHOD_ACK			 "ACK"
#define METHOD_BYE			 "BYE"
#define METHOD_INFO			 "INFO"
#define METHOD_REGISTER		 "REGISTER"
#define METHOD_OPTIONS		 "OPTIONS"
#define METHOD_DESCRIBE		 "DESCRIBE"
#define METHOD_ANNOUNCE		 "ANNOUNCE"
#define METHOD_SETUP		 "SETUP"
#define METHOD_PLAY			 "PLAY"
#define METHOD_PAUSE		 "PAUSE"
#define METHOD_TEARDOWN		 "TEARDOWN"
#define METHOD_GET_PARAMETER  "GET_PARAMETER"
#define METHOD_SET_PARAMETER  "SET_PARAMETER"
#define METHOD_REDIRECT		 "REDIRECT"
#define METHOD_RECORD		 "RECORD"




#define DHTTP_SIP_VERSION		"SIP/2.0"
#define DHTTP_HTTP_VERSION		"HTTP/1.1"
#define DHTTP_RTSP_VERSION		"RTSP/1.0"

//HTTP Head Key
#define DHTTP_CONTENT_TYPE		"Content-Type"
#define		DHTTP_CONTENT_TYPE_XML			"text/xml"
#define		DHTTP_CONTENT_TYPE_HTML			"text/html"
#define		DHTTP_CONTENT_TYPE_SDP			"application/sdp"
#define		DHTTP_CONTENT_TYPE_HTTP			"application/http"
#define		DHTTP_CONTENT_TYPE_OCTET		"application/octet-stream"
#define		DHTTP_CONTENT_TYPE_URLENCODED	"application/x-www-form-urlencoded"
#define		DHTTP_CONTENT_TYPE_JPG			"image/jpeg"
#define		DHTTP_CONTENT_TYPE_JSON			"application/json"


#define DHTTP_USER_AGENT		"User-Agent"
#define DHTTP_HOST				"Host"
#define DHTTP_X_CLIENT_ADDRESS	"X-Client-Address"
#define DHTTP_X_TRANSACTION_ID	"X-Transaction-ID"
#define DHTTP_CONTENT_LENGTH	"Content-Length"
#define DHTTP_ACCEPT_ENCODING	"Accept-Encoding"
#define DHTTP_ACCEPT_LANGUAGE	"Accept-Language"
#define DHTTP_ALLOW				"Allow"
#define DHTTP_BANDWIDTH			"Bandwidth"
#define	DHTTP_BLOCKSIZE			"Blocksize"
#define DHTTP_CONFERENCE		"Conference"
#define DHTTP_CONNECTION		"Connection"
#define DHTTP_CONTENT_BASE		"Content-Base"
#define DHTTP_CONTENT_ENCODING	"Content-Encoding"
#define DHTTP_CONTENT_LANGUAGE	"Content-Language"
#define DHTTP_ACCEPT			"Accept"
#define DHTTP_VIA				"Via"
#define DHTTP_DATE				"Date"
#define DHTTP_SERVER			"Server"
#define DHTTP_SET_COOKIE		"Set-Cookie"
#define DHTTP_COOKIE			"Cookie"
#define DHTTP_FROM				"From"
#define DHTTP_TO				"To"
#define DHTTP_CONTACT			"Contact"
#define DHTTP_CSEQ				"CSeq"
#define DHTTP_CALLID			"Call-ID"
#define DHTTP_MAX_FORWARDS		"Max-Forwards"
#define DHTTP_RANGE				"Range"
#define DHTTP_RTP_INFO			"RTP-Info"
#define DHTTP_SCALE				"Scale"
#define DHTTP_SPEED				"Speed"
#define DHTTP_SESSION			"Session"
#define DHTTP_TIMESTAMP			"Timestamp"
#define DHTTP_TRANSPORT			"Transport"
#define DHTTP_WWW_AUTHENTICATE	"WWW-Authenticate"
#define DHTTP_LAST_MODIFIED		"Last-Modified"
#define DHTTP_EXPIRES			"Expires"
#define DHTTP_VARY				"Vary"
#define DHTTP_UNSUPPORT			"Unsupported"

#endif  // INCLUDED_LIBDSL_DHTTPPROTOCOL_H

