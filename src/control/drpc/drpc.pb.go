// Code generated by protoc-gen-go. DO NOT EDIT.
// source: drpc.proto

package drpc

import (
	fmt "fmt"
	math "math"

	proto "github.com/golang/protobuf/proto"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// *
// Status represents the valid values for a response status.
type Status int32

const (
	Status_SUCCESS        Status = 0
	Status_SUBMITTED      Status = 1
	Status_FAILURE        Status = 2
	Status_UNKNOWN_MODULE Status = 3
	Status_UNKNOWN_METHOD Status = 4
)

var Status_name = map[int32]string{
	0: "SUCCESS",
	1: "SUBMITTED",
	2: "FAILURE",
	3: "UNKNOWN_MODULE",
	4: "UNKNOWN_METHOD",
}

var Status_value = map[string]int32{
	"SUCCESS":        0,
	"SUBMITTED":      1,
	"FAILURE":        2,
	"UNKNOWN_MODULE": 3,
	"UNKNOWN_METHOD": 4,
}

func (x Status) String() string {
	return proto.EnumName(Status_name, int32(x))
}

func (Status) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_e4c8b3f839895dc3, []int{0}
}

// *
// Call is a structure outlining a function call to be executed
// over a drpc channel.
//
// module is the numerical identifier for the drpc module to process the call
// method is the specific method within the module
// sequence is the internal sequence counter for matching calls to responses
// body is the opaque data of the function call arguments
type Call struct {
	Module               int32    `protobuf:"varint,1,opt,name=module,proto3" json:"module,omitempty"`
	Method               int32    `protobuf:"varint,2,opt,name=method,proto3" json:"method,omitempty"`
	Sequence             int64    `protobuf:"varint,3,opt,name=sequence,proto3" json:"sequence,omitempty"`
	Body                 []byte   `protobuf:"bytes,4,opt,name=body,proto3" json:"body,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Call) Reset()         { *m = Call{} }
func (m *Call) String() string { return proto.CompactTextString(m) }
func (*Call) ProtoMessage()    {}
func (*Call) Descriptor() ([]byte, []int) {
	return fileDescriptor_e4c8b3f839895dc3, []int{0}
}

func (m *Call) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Call.Unmarshal(m, b)
}
func (m *Call) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Call.Marshal(b, m, deterministic)
}
func (m *Call) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Call.Merge(m, src)
}
func (m *Call) XXX_Size() int {
	return xxx_messageInfo_Call.Size(m)
}
func (m *Call) XXX_DiscardUnknown() {
	xxx_messageInfo_Call.DiscardUnknown(m)
}

var xxx_messageInfo_Call proto.InternalMessageInfo

func (m *Call) GetModule() int32 {
	if m != nil {
		return m.Module
	}
	return 0
}

func (m *Call) GetMethod() int32 {
	if m != nil {
		return m.Method
	}
	return 0
}

func (m *Call) GetSequence() int64 {
	if m != nil {
		return m.Sequence
	}
	return 0
}

func (m *Call) GetBody() []byte {
	if m != nil {
		return m.Body
	}
	return nil
}

// *
// Response is the data to the Call with a given sequence number.
//
// sequence is the sequence number of the drpc call for the response.
// status represents the return/faulure value of the call
// body represents the returned data if a call returns more than just a status.
type Response struct {
	Sequence             int64    `protobuf:"varint,1,opt,name=sequence,proto3" json:"sequence,omitempty"`
	Status               Status   `protobuf:"varint,2,opt,name=status,proto3,enum=drpc.Status" json:"status,omitempty"`
	Body                 []byte   `protobuf:"bytes,3,opt,name=body,proto3" json:"body,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Response) Reset()         { *m = Response{} }
func (m *Response) String() string { return proto.CompactTextString(m) }
func (*Response) ProtoMessage()    {}
func (*Response) Descriptor() ([]byte, []int) {
	return fileDescriptor_e4c8b3f839895dc3, []int{1}
}

func (m *Response) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Response.Unmarshal(m, b)
}
func (m *Response) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Response.Marshal(b, m, deterministic)
}
func (m *Response) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Response.Merge(m, src)
}
func (m *Response) XXX_Size() int {
	return xxx_messageInfo_Response.Size(m)
}
func (m *Response) XXX_DiscardUnknown() {
	xxx_messageInfo_Response.DiscardUnknown(m)
}

var xxx_messageInfo_Response proto.InternalMessageInfo

func (m *Response) GetSequence() int64 {
	if m != nil {
		return m.Sequence
	}
	return 0
}

func (m *Response) GetStatus() Status {
	if m != nil {
		return m.Status
	}
	return Status_SUCCESS
}

func (m *Response) GetBody() []byte {
	if m != nil {
		return m.Body
	}
	return nil
}

func init() {
	proto.RegisterEnum("drpc.Status", Status_name, Status_value)
	proto.RegisterType((*Call)(nil), "drpc.Call")
	proto.RegisterType((*Response)(nil), "drpc.Response")
}

func init() { proto.RegisterFile("drpc.proto", fileDescriptor_e4c8b3f839895dc3) }

var fileDescriptor_e4c8b3f839895dc3 = []byte{
	// 241 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0xe2, 0x4a, 0x29, 0x2a, 0x48,
	0xd6, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0x62, 0x01, 0xb1, 0x95, 0xd2, 0xb8, 0x58, 0x9c, 0x13,
	0x73, 0x72, 0x84, 0xc4, 0xb8, 0xd8, 0x72, 0xf3, 0x53, 0x4a, 0x73, 0x52, 0x25, 0x18, 0x15, 0x18,
	0x35, 0x58, 0x83, 0xa0, 0x3c, 0xb0, 0x78, 0x6a, 0x49, 0x46, 0x7e, 0x8a, 0x04, 0x13, 0x54, 0x1c,
	0xcc, 0x13, 0x92, 0xe2, 0xe2, 0x28, 0x4e, 0x2d, 0x2c, 0x4d, 0xcd, 0x4b, 0x4e, 0x95, 0x60, 0x56,
	0x60, 0xd4, 0x60, 0x0e, 0x82, 0xf3, 0x85, 0x84, 0xb8, 0x58, 0x92, 0xf2, 0x53, 0x2a, 0x25, 0x58,
	0x14, 0x18, 0x35, 0x78, 0x82, 0xc0, 0x6c, 0xa5, 0x04, 0x2e, 0x8e, 0xa0, 0xd4, 0xe2, 0x82, 0xfc,
	0xbc, 0xe2, 0x54, 0x14, 0xbd, 0x8c, 0x68, 0x7a, 0x55, 0xb8, 0xd8, 0x8a, 0x4b, 0x12, 0x4b, 0x4a,
	0x8b, 0xc1, 0xf6, 0xf1, 0x19, 0xf1, 0xe8, 0x81, 0x9d, 0x1c, 0x0c, 0x16, 0x0b, 0x82, 0xca, 0xc1,
	0x6d, 0x60, 0x46, 0xd8, 0xa0, 0x15, 0xc9, 0xc5, 0x06, 0x51, 0x25, 0xc4, 0xcd, 0xc5, 0x1e, 0x1c,
	0xea, 0xec, 0xec, 0x1a, 0x1c, 0x2c, 0xc0, 0x20, 0xc4, 0xcb, 0xc5, 0x19, 0x1c, 0xea, 0xe4, 0xeb,
	0x19, 0x12, 0xe2, 0xea, 0x22, 0xc0, 0x08, 0x92, 0x73, 0x73, 0xf4, 0xf4, 0x09, 0x0d, 0x72, 0x15,
	0x60, 0x12, 0x12, 0xe2, 0xe2, 0x0b, 0xf5, 0xf3, 0xf6, 0xf3, 0x0f, 0xf7, 0x8b, 0xf7, 0xf5, 0x77,
	0x09, 0xf5, 0x71, 0x15, 0x60, 0x46, 0x11, 0x73, 0x0d, 0xf1, 0xf0, 0x77, 0x11, 0x60, 0x49, 0x62,
	0x03, 0x87, 0x98, 0x31, 0x20, 0x00, 0x00, 0xff, 0xff, 0x25, 0xe4, 0xe4, 0x71, 0x3f, 0x01, 0x00,
	0x00,
}
