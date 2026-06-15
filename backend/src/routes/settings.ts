import { Router } from "express";
import { JsonStore } from "../services/storage.service.js";
import { PanelGatewayService } from "../services/panel-gateway.service.js";
import type { Settings } from "../types/domain.js";

export function settingsRouter(store: JsonStore<Settings>, panel: PanelGatewayService) {
  const router = Router();
  router.get("/", async (_req, res) => res.json(await store.get()));
  router.put("/", async (req, res) => {
    const current = await store.get();
    const next = { ...current, ...req.body, avatar: { ...current.avatar, ...req.body.avatar }, ai: { ...current.ai, ...req.body.ai }, general: { ...current.general, ...req.body.general } };
    await store.set(next); panel.sync(); res.json(next);
  });
  return router;
}
