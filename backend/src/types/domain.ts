export type Profile = { id: string; name: string; age: number; avatarId?: string; createdAt: string };
export type Teacher = { id: string; name: string; subject: string; avatarId?: string; profileId?: string; createdAt: string };
export type MaterialType = "url" | "txt" | "pdf" | "docx";
export type Material = { id: string; teacherId?: string; subject?: string; title: string; type: MaterialType; source: string; content: string; createdAt: string };
export type Settings = {
  avatar: { selectedAvatarId: string; gender: string; voice: string; mouthAnimationEnabled: boolean; showOnTeacherCard: boolean };
  ai: { assistantName: string; model: string; language: "sk" | "en" | "auto"; responseLength: "short" | "medium" | "long"; temperature: number; useMaterials: boolean; modes: string[]; customInstructions: string };
  general: { wifiSsid: string; wifiPassword: string; backendHost: string; backendPort: number; bluetoothEnabled: boolean; wakeOnTouch: boolean; autoReconnect: boolean; autoSleepSeconds: number };
};
