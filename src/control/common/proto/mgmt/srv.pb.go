// Code generated by protoc-gen-go. DO NOT EDIT.
// source: srv.proto

package mgmt

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type DaosRequestStatus int32

const (
	DaosRequestStatus_SUCCESS          DaosRequestStatus = 0
	DaosRequestStatus_ERR_UNKNOWN      DaosRequestStatus = -1
	DaosRequestStatus_ERR_INVALID_RANK DaosRequestStatus = -2
	DaosRequestStatus_ERR_INVALID_UUID DaosRequestStatus = -3
)

var DaosRequestStatus_name = map[int32]string{
	0:  "SUCCESS",
	-1: "ERR_UNKNOWN",
	-2: "ERR_INVALID_RANK",
	-3: "ERR_INVALID_UUID",
}
var DaosRequestStatus_value = map[string]int32{
	"SUCCESS":          0,
	"ERR_UNKNOWN":      -1,
	"ERR_INVALID_RANK": -2,
	"ERR_INVALID_UUID": -3,
}

func (x DaosRequestStatus) String() string {
	return proto.EnumName(DaosRequestStatus_name, int32(x))
}
func (DaosRequestStatus) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_srv_0efc7dbf5aaedc97, []int{0}
}

// Identifier for server rank within DAOS pool
type DaosRank struct {
	PoolUuid             string   `protobuf:"bytes,1,opt,name=pool_uuid,json=poolUuid,proto3" json:"pool_uuid,omitempty"`
	Rank                 uint32   `protobuf:"varint,2,opt,name=rank,proto3" json:"rank,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *DaosRank) Reset()         { *m = DaosRank{} }
func (m *DaosRank) String() string { return proto.CompactTextString(m) }
func (*DaosRank) ProtoMessage()    {}
func (*DaosRank) Descriptor() ([]byte, []int) {
	return fileDescriptor_srv_0efc7dbf5aaedc97, []int{0}
}
func (m *DaosRank) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_DaosRank.Unmarshal(m, b)
}
func (m *DaosRank) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_DaosRank.Marshal(b, m, deterministic)
}
func (dst *DaosRank) XXX_Merge(src proto.Message) {
	xxx_messageInfo_DaosRank.Merge(dst, src)
}
func (m *DaosRank) XXX_Size() int {
	return xxx_messageInfo_DaosRank.Size(m)
}
func (m *DaosRank) XXX_DiscardUnknown() {
	xxx_messageInfo_DaosRank.DiscardUnknown(m)
}

var xxx_messageInfo_DaosRank proto.InternalMessageInfo

func (m *DaosRank) GetPoolUuid() string {
	if m != nil {
		return m.PoolUuid
	}
	return ""
}

func (m *DaosRank) GetRank() uint32 {
	if m != nil {
		return m.Rank
	}
	return 0
}

type DaosResponse struct {
	Status               DaosRequestStatus `protobuf:"varint,1,opt,name=status,proto3,enum=mgmt.DaosRequestStatus" json:"status,omitempty"`
	XXX_NoUnkeyedLiteral struct{}          `json:"-"`
	XXX_unrecognized     []byte            `json:"-"`
	XXX_sizecache        int32             `json:"-"`
}

func (m *DaosResponse) Reset()         { *m = DaosResponse{} }
func (m *DaosResponse) String() string { return proto.CompactTextString(m) }
func (*DaosResponse) ProtoMessage()    {}
func (*DaosResponse) Descriptor() ([]byte, []int) {
	return fileDescriptor_srv_0efc7dbf5aaedc97, []int{1}
}
func (m *DaosResponse) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_DaosResponse.Unmarshal(m, b)
}
func (m *DaosResponse) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_DaosResponse.Marshal(b, m, deterministic)
}
func (dst *DaosResponse) XXX_Merge(src proto.Message) {
	xxx_messageInfo_DaosResponse.Merge(dst, src)
}
func (m *DaosResponse) XXX_Size() int {
	return xxx_messageInfo_DaosResponse.Size(m)
}
func (m *DaosResponse) XXX_DiscardUnknown() {
	xxx_messageInfo_DaosResponse.DiscardUnknown(m)
}

var xxx_messageInfo_DaosResponse proto.InternalMessageInfo

func (m *DaosResponse) GetStatus() DaosRequestStatus {
	if m != nil {
		return m.Status
	}
	return DaosRequestStatus_SUCCESS
}

func init() {
	proto.RegisterType((*DaosRank)(nil), "mgmt.DaosRank")
	proto.RegisterType((*DaosResponse)(nil), "mgmt.DaosResponse")
	proto.RegisterEnum("mgmt.DaosRequestStatus", DaosRequestStatus_name, DaosRequestStatus_value)
}

func init() { proto.RegisterFile("srv.proto", fileDescriptor_srv_0efc7dbf5aaedc97) }

var fileDescriptor_srv_0efc7dbf5aaedc97 = []byte{
	// 216 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0xe2, 0x2c, 0x2e, 0x2a, 0xd3,
	0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0x62, 0xc9, 0x4d, 0xcf, 0x2d, 0x51, 0xb2, 0xe6, 0xe2, 0x70,
	0x49, 0xcc, 0x2f, 0x0e, 0x4a, 0xcc, 0xcb, 0x16, 0x92, 0xe6, 0xe2, 0x2c, 0xc8, 0xcf, 0xcf, 0x89,
	0x2f, 0x2d, 0xcd, 0x4c, 0x91, 0x60, 0x54, 0x60, 0xd4, 0xe0, 0x0c, 0xe2, 0x00, 0x09, 0x84, 0x96,
	0x66, 0xa6, 0x08, 0x09, 0x71, 0xb1, 0x14, 0x25, 0xe6, 0x65, 0x4b, 0x30, 0x29, 0x30, 0x6a, 0xf0,
	0x06, 0x81, 0xd9, 0x4a, 0xf6, 0x5c, 0x3c, 0x60, 0xcd, 0xa9, 0xc5, 0x05, 0xf9, 0x79, 0xc5, 0xa9,
	0x42, 0xfa, 0x5c, 0x6c, 0xc5, 0x25, 0x89, 0x25, 0xa5, 0xc5, 0x60, 0xdd, 0x7c, 0x46, 0xe2, 0x7a,
	0x20, 0x3b, 0xf4, 0x20, 0x6a, 0x0a, 0x4b, 0x53, 0x8b, 0x4b, 0x82, 0xc1, 0xd2, 0x41, 0x50, 0x65,
	0x5a, 0x15, 0x5c, 0x82, 0x18, 0x92, 0x42, 0xdc, 0x5c, 0xec, 0xc1, 0xa1, 0xce, 0xce, 0xae, 0xc1,
	0xc1, 0x02, 0x0c, 0x42, 0x12, 0x5c, 0xdc, 0xae, 0x41, 0x41, 0xf1, 0xa1, 0x7e, 0xde, 0x7e, 0xfe,
	0xe1, 0x7e, 0x02, 0xff, 0x61, 0x80, 0x51, 0x48, 0x96, 0x4b, 0x00, 0x24, 0xe3, 0xe9, 0x17, 0xe6,
	0xe8, 0xe3, 0xe9, 0x12, 0x1f, 0xe4, 0xe8, 0xe7, 0x2d, 0xf0, 0x0f, 0xa7, 0x74, 0x68, 0xa8, 0xa7,
	0x8b, 0xc0, 0x5f, 0xb8, 0x74, 0x12, 0x1b, 0x38, 0x10, 0x8c, 0x01, 0x01, 0x00, 0x00, 0xff, 0xff,
	0xe4, 0x29, 0xad, 0xff, 0x11, 0x01, 0x00, 0x00,
}
