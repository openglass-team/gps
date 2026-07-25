export interface GPSCoords {导出 接口 GPSCoords {
  lat: number;
  lng: number;  经度: 数字;
}

export interface RouteStep {导出 接口 RouteStep {导出 接口 RouteStep {导出 接口 RouteStep {
  distance: number;距离: 数字;
  duration持续时间: number;
  instruction: string;
  name: string;  名称: 字符串;  name名称: string字符串;  名称: 字符串;
}

export interface RouteResult {
  totalDistance总距离: number;
  totalDuration: number;
  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];  steps: RouteStep[];步骤: RouteStep[];  步骤: RouteStep[];步骤: RouteStep[];
}

export interface POIResult {
  name名称: string字符串;  名称: 字符串;  name名称: string字符串;  名称: 字符串;  name名称: string字符串;  名称: 字符串;  name名称: string字符串;  名称: 字符串;
  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;  lat: number;纬度: 数字;  纬度: 数字;纬度: 数字;
  lng: number;经度: 数字;  经度: 数字;经度: 数字;
  displayName: string;displayName显示名称: 字符串;
}

export async function getCurrentAddress(lat: number, lng: number): Promise<string> {导出 异步 函数(纬度: 数字, 经度: 数字): Promise<字符串> {
  const url = `https://nominatim.openstreetmap.org/reverse?format=json&lat=${lat}&lon=${lng}&addressdetails=1&accept-language=zh`;
  const res = await fetch(url, {
    headers: { 'User-Agent': 'OpenGlass/1.0' }
  });
  const data = await res.json();
  return data.display_name || 'Unknown location';return data.display_name || '未知位置';
}

export async function searchNearby(lat: number, lng: number, query: string): Promise<POIResult[]> {异步函数 searchNearby(lat: number, lng: number, query: string): Promise<POIResult[]> {
  const url = `https://nominatim.openstreetmap.org/search?q=${encodeURIComponent(query)}&format=json&limit=5&lat=${lat}&lon=${lng}&bounded=1&addressdetails=1&accept-language=zh`;
  const res = await fetch(url, {
    headers: { 'User-Agent': 'OpenGlass/1.0' }
  });
  const data = await res.json();
  return data.map((item: any) => ({
    name: item.name || item.display_name?.split(',')[0] || 'Unknown',
    lat: parseFloat(item.lat),
    lng: parseFloat(item.lon),
    displayName: item.display_name,
  }));
}

export async function getRoute(from: GPSCoords, to: GPSCoords): Promise<RouteResult> {
  const url = `https://router.project-osrm.org/route/v1/foot/${from.lng},${from.lat};${to.lng},${to.lat}?steps=true&geometries=geojson&overview=full`;
  const res = await fetch(url);
  const data = await res.json();

  if (!data.routes || data.routes.length === 0) {
    return { totalDistance: 0, totalDuration: 0, steps: [] };
  }

  const route = data.routes[0];
  const legs = route.legs[0];

  const steps: RouteStep[] = legs.steps.map((s: any) => ({
    distance: s.distance,
    duration: s.duration,
    instruction: s.maneuver?.type || 'straight',
    name: s.name || '',
  }));

  return {
    totalDistance: route.distance,
    totalDuration: route.duration,
    steps,
  };
}

export function formatRouteSteps(route: RouteResult): string {
  if (route.steps.length === 0) return 'No route found';

  const maneuverMap: Record<string, string> = {
    'turn': '转弯',
    'new name': '进入',
    'depart': '出发',
    'arrive': '到达目的地',
    'continue': '继续直行',
    'roundabout': '进入环岛',
    'rotary': '进入环岛',
    'end of road': '在路尽头',
    'fork': '在岔路口',
    'merge': '并道',
    'off ramp': '下匝道',
    'on ramp': '上匝道',
    'slight left': '稍向左转',
    'slight right': '稍向右转',
    'sharp left': '急左转',
    'sharp right': '急右转',
    'uturn': '调头',
  };

  let text = '';
  const maxSteps = Math.min(route.steps.length, 5);
  for (let i = 0; i < maxSteps; i++) {
    const step = route.steps[i];
    const dist = step.distance < 1000
      ? `${Math.round(step.distance)}米`
      : `${(step.distance / 1000).toFixed(1)}公里`;

    const action = maneuverMap[step.instruction] || step.instruction;
    const road = step.name ? ` ${step.name}` : '';
    text += `${action}${road}，${dist}后`;

    if (i < maxSteps - 1) {
      text += '；';
    }
  }

  text += `。全程约${Math.round(route.totalDistance)}米，步行约${Math.round(route.totalDuration / 60)}分钟。`;
  return text;
}

export async function navigateTo(
  current: GPSCoords,
  destination: string
): Promise<{ route: RouteResult; poi: POIResult; instructions: string } | null> {
  const pois = await searchNearby(current.lat, current.lng, destination);
  if (pois.length === 0) return null;

  const target = pois[0];
  const route = await getRoute(current, { lat: target.lat, lng: target.lng });
  const instructions = formatRouteSteps(route);

  return { route, poi: target, instructions };
}
