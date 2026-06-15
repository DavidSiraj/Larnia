import { Router } from "express";
import { z } from "zod";
import { AIService } from "../services/ai.service.js";
import { JsonStore } from "../services/storage.service.js";
import { PanelGatewayService } from "../services/panel-gateway.service.js";
import type { Teacher, Material, Settings } from "../types/domain.js";

export function chatRouter(ai: AIService, teachers: JsonStore<Teacher[]>, materials: JsonStore<Material[]>, settings: JsonStore<Settings>, panel: PanelGatewayService) {
  const router = Router();
  router.post("/", async (req, res, next) => {
    try {
      const body = z.object({ teacherId: z.string().min(1), question: z.string().min(1) }).parse(req.body);
      const teacher = (await teachers.get()).find(x => x.id === body.teacherId);
      if (!teacher) return res.status(404).json({ error: "Teacher not found" });
      const cfg = await settings.get();
      const relevant = (await materials.get()).filter(m => !m.teacherId || m.teacherId === teacher.id || m.subject === teacher.subject);
      panel.setTalking(true);
      const answer = await ai.answer(body.question, teacher, cfg, relevant);
      panel.showAnswer(answer, teacher.avatarId || cfg.avatar.selectedAvatarId);
      panel.setTalking(false);
      res.json({ answer, teacher });
    } catch (e) { panel.setTalking(false); next(e); }
  });
  return router;
}
