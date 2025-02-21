#include "Map.h"
#include "App.h"
#include "Render.h"
#include "Scene.h"
#include "World.h"
#include "Collider.h"
#include "LilipadPuzzle.h"
#include "StonePuzzle.h"
#include "ButtonsPuzzle.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>

Map::Map() : Entity(EntityType::MAP)
{
	mapLoaded = false;
	folder.Create("Assets/Maps");
}

// Destructor
Map::~Map()
{}

// L06: DONE 7: Ask for the value of a custom property
int Properties::GetProperty(const char* value, int defaultValue) const
{
	ListItem<Property*>* item = list.start;

	while (item)
	{
		if (item->data->name == value)
			return item->data->value;
		item = item->next;
	}

	return defaultValue;
}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
    LOG("Loading Map Parser");
    bool ret = true;

    //folder.Create(config.child("folder").child_value());

    return ret;
}

// L12a: Methods not required anymore -> Using PathFinding class
/*
bool Map::Start()
{
	tileX = app->tex->Load("Assets/maps/x.png");

	return true;
}

void Map::ResetPath(iPoint start)
{
	frontier.Clear();
	visited.Clear();
    breadcrumbs.Clear();
    
	frontier.Push(start, 0);
	visited.Add(start);
    breadcrumbs.Add(start);

	memset(costSoFar, 0, sizeof(uint) * COST_MAP_SIZE * COST_MAP_SIZE);
}

void Map::DrawPath()
{
	iPoint point;

	// Draw visited
	ListItem<iPoint>* item = visited.start;

	while(item)
	{
		point = item->data;
		TileSet* tileset = GetTilesetFromTileId(26);

		SDL_Rect rec = tileset->GetTileRect(26);
		iPoint pos = MapToWorld(point.x, point.y);

		app->render->DrawTexture(tileset->texture, pos.x, pos.y, &rec);

		item = item->next;
	}

	// Draw frontier
	for (uint i = 0; i < frontier.Count(); ++i)
	{
		point = *(frontier.Peek(i));
		TileSet* tileset = GetTilesetFromTileId(25);

		SDL_Rect rec = tileset->GetTileRect(25);
		iPoint pos = MapToWorld(point.x, point.y);

		app->render->DrawTexture(tileset->texture, pos.x, pos.y, &rec);
	}
    
    // Draw path
	for (uint i = 0; i < path.Count(); ++i)
	{
		iPoint pos = MapToWorld(path[i].x, path[i].y);
		app->render->DrawTexture(tileX, pos.x, pos.y);
	}
}

int Map::MovementCost(int x, int y) const
{
	int ret = -1;

	if ((x >= 0) && (x < data.width) && (y >= 0) && (y < data.height))
	{
		int id = data.layers.start->next->data->Get(x, y);

		if (id == 0) ret = 3;
		else ret = 0;
	}

	return ret;
}

void Map::ComputePath(int x, int y)
{
	path.Clear();
	iPoint goal = WorldToMap(x, y);

	// L11: DONE 2: Follow the breadcrumps to goal back to the origin
	// add each step into "path" dyn array (it will then draw automatically)
	path.PushBack(goal);
	int index = visited.Find(goal);

	while ((index >= 0) && (goal != breadcrumbs[index]))
	{
		goal = breadcrumbs[index];
		path.PushBack(goal);
		index = visited.Find(goal);
	}
}

void Map::ComputePathAStar(int x, int y)
{
	// L12a: Compute AStart pathfinding
}


void Map::PropagateBFS()
{
	// L10: DONE 1: If frontier queue contains elements
	// pop the last one and calculate its 4 neighbors
	iPoint curr;
	if (frontier.Pop(curr))
	{
		// L10: DONE 2: For each neighbor, if not visited, add it
		// to the frontier queue and visited list
		iPoint neighbors[4];
		neighbors[0].Create(curr.x + 1, curr.y + 0);
		neighbors[1].Create(curr.x + 0, curr.y + 1);
		neighbors[2].Create(curr.x - 1, curr.y + 0);
		neighbors[3].Create(curr.x + 0, curr.y - 1);

		for (uint i = 0; i < 4; ++i)
		{
			if (MovementCost(neighbors[i].x, neighbors[i].y) > 0)
			{
				if (visited.Find(neighbors[i]) == -1)
				{
					frontier.Push(neighbors[i], 0);
					visited.Add(neighbors[i]);

					// L11: DONE 1: Record the direction to the previous node 
					// with the new list "breadcrumps"
					breadcrumbs.Add(curr);
				}
			}
		}
	}
}

void Map::PropagateDijkstra()
{
	// L11: TODO 3: Taking BFS as a reference, implement the Dijkstra algorithm
	// use the 2 dimensional array "costSoFar" to track the accumulated costs
	// on each cell (is already reset to 0 automatically)
}

void Map::PropagateAStar(int heuristic)
{
	// L12a: TODO 2: Implement AStar algorythm
	// Consider the different heuristics
}
*/

void Map::UpdateLayers()
{
	data.orgLayers.Clear();
	List<int> list;
	for (int i = 0; i < data.layers.Count(); i++) list.Add(data.layers[i]->properties.GetProperty("y"));
	list.BubbleSort();
	for (int i = 0; i < list.Count(); i++) data.orgLayers.PushBack(GetLayerFromY(list[i]));
	//data.orgLayers.Flip();
}

// Draw the map (all requried layers)
void Map::Draw()
{
	Player* p = app->scene->player1;
	World* w = app->scene->world;
	bool playerDraw = false;
	if (mapLoaded == false) return;

	camOffset.x = app->render->camera.x;
	camOffset.y = app->render->camera.y;

	UpdateLayers();

	if (app->scene->world->GetPlace() != Places::HOUSE && app->scene->world->GetPlace() != Places::TAVERN)
	{
		// L06: DONE 4: Make sure we draw all the layers and not just the first one
		for (int i = 0; i < data.orgLayers.Count(); i++)
		{
			MapLayer* layer = data.orgLayers[i];
			if (strcmp(layer->name.GetString(), "colliders") != 0)
			{
				if (layer->properties.GetProperty("y") * SCALE < p->collisionRect.y)
				{
					if ((layer->properties.GetProperty("drawable", 1) != 0) || drawColliders) DrawLayer(i);
				}
				else
				{
					if (!playerDraw)
					{
						if (app->scene->world->GetPlace() == Places::GRASSY_LAND_2)
						{
							w->lilipadPuzzle1->Draw();
							w->stonePuzzle1->Draw();
							w->buttonPuzzle1->Draw();
						}

						if (app->scene->world->GetPlace() == Places::GOLEM_STONES)
						{
							bool yes1 = false;
							bool yes2 = false;
							if (app->scene->player1->collisionRect.y + app->scene->player1->collisionRect.h >= (17 * 28))
							{
								if (!app->scene->bossTBeat) app->render->DrawTexture(w->tutorialRockTex, w->tutorialBossRect.x, w->tutorialBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->tutorialBossRect.x, w->tutorialBossRect.y - 95, 2.9f, false);

								if (!app->scene->boss2Beat) app->render->DrawTexture(w->thirdRockTex, w->thirdBossRect.x, w->thirdBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->thirdBossRect.x, w->thirdBossRect.y - 95, 2.9f, false);
								yes1 = true;
							}
							if (app->scene->player1->collisionRect.y + app->scene->player1->collisionRect.h >= (14 * 28))
							{
								if (!app->scene->boss1Beat) app->render->DrawTexture(w->secondRockTex, w->secondBossRect.x, w->secondBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->secondBossRect.x, w->secondBossRect.y - 95, 2.9f, false);
								yes2 = true;
							}

							w->DrawNPC();
							w->DrawEnemy();
							w->DrawStomps();
							w->DrawPlayer();
							playerDraw = true;
							i--;

							if (!yes1)
							{
								if (!app->scene->bossTBeat) app->render->DrawTexture(w->tutorialRockTex, w->tutorialBossRect.x, w->tutorialBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->tutorialBossRect.x, w->tutorialBossRect.y - 95, 2.9f, false);

								if (!app->scene->boss2Beat) app->render->DrawTexture(w->thirdRockTex, w->thirdBossRect.x, w->thirdBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->thirdBossRect.x, w->thirdBossRect.y - 95, 2.9f, false);
							}
							if (!yes2)
							{
								if (!app->scene->boss1Beat) app->render->DrawTexture(w->secondRockTex, w->secondBossRect.x, w->secondBossRect.y - 95, 2.9f, false);
								else app->render->DrawTexture(w->unableRockTex, w->secondBossRect.x, w->secondBossRect.y - 95, 2.9f, false);
							}
						}
						else
						{
							if (app->scene->world->GetPlace() == Places::CAVE) app->render->DrawTexture(w->caveWallTex, -1, -512, 1.01f, 1, false);
							w->DrawNPC();
							w->DrawEnemy();
							w->DrawStomps();
							w->DrawPlayer();
							playerDraw = true;
							i--;
						}
					}
					else
					{
						if ((layer->properties.GetProperty("drawable", 1) != 0) || drawColliders) DrawLayer(i);
					}
				}
			}
			layer = nullptr;
		}
	}
	else
	{
		for (int i = 0; i < data.layers.Count(); i++)
		{
			if (strcmp(data.layers[i]->name.GetString(), "colliders") != 0)
			{
				if ((data.layers[i]->properties.GetProperty("drawable", 1) != 0) || drawColliders) DrawLayer(i);
			}
		}
	}

	p = nullptr;
}

void Map::DrawLayer(int num)
{
	if (num < data.layers.Count())
	{
		app->render->scale = scale;

		// L04: DONE 5: Prepare the loop to draw all tilesets + DrawTexture()
		for (int y = 0; y < data.height; ++y)
		{
			for (int x = 0; x < data.width; ++x)
			{
				//CANVIAR
				int tileId = 0;
				if (app->scene->world->GetPlace() != Places::HOUSE && app->scene->world->GetPlace() != Places::TAVERN) tileId = data.orgLayers[num]->Get(x, y);
				else
				{
					tileId = data.layers[num]->Get(x, y);
				}
				//-----

				if (tileId > 0)
				{
					// L04: DONE 9: Complete the draw function
					TileSet* tileset = GetTilesetFromTileId(tileId);

					SDL_Rect rec = GetTilesetFromTileId(tileId)->GetTileRect(tileId);
					iPoint pos = MapToWorld(x, y);

					SDL_Rect invertCam = { -app->render->camera.x, -app->render->camera.y, app->render->camera.w, app->render->camera.h };

					if (colliderUtils.CheckCollision(invertCam, { (int)((pos.x + tileset->offsetX) * 1.75), (int)((pos.y + tileset->offsetY) * 1.75), (int)(16 * 1.75), (int)(16 * 1.75) })) app->render->DrawTexture(tileset->texture, pos.x + tileset->offsetX, pos.y + tileset->offsetY, SCALE, &rec);

					tileset = nullptr;
				}
			}
		}
		app->render->scale = 1;
	}
}

// L04: DONE 8: Create a method that translates x,y coordinates 
// from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	// L05: DONE 1: Add isometric map to world coordinates
	if (data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * data.tileWidth;
		ret.y = y * data.tileHeight;
	}
	else if (data.type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (data.tileWidth / 2);
		ret.y = (x + y) * (data.tileHeight / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

// L05: DONE 2: Add orthographic world to map coordinates
iPoint Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	// L05: DONE 3: Add the case for isometric maps to WorldToMap
	if (data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / data.tileWidth;
		ret.y = y / data.tileHeight;
	}
	else if (data.type == MAPTYPE_ISOMETRIC)
	{

		float halfWidth = data.tileWidth * 0.5f;
		float halfHeight = data.tileHeight * 0.5f;
		ret.x = int((x / halfWidth + y / halfHeight) / 2);
		ret.y = int((y / halfHeight - (x / halfWidth)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

SDL_Rect Map::GetTilemapRec(int x, int y) const
{
	iPoint pos = MapToWorld(x, y);
	SDL_Rect rec = { pos.x * scale + camOffset.x, pos.y * scale + camOffset.y, 
					 data.tileWidth * scale, data.tileHeight * scale };

	return rec;
}

MapLayer* Map::GetLayerFromY(int y) const
{
	ListItem<MapLayer*>* item = data.layers.start;
	MapLayer* set = item->data;

	while (item)
	{
		if (y == item->data->properties.GetProperty("y"))
		{
			set = item->data;
			break;
		}
		set = item->data;
		item = item->next;
	}

	return set;
}

// L06: DONE 3: Pick the right Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int id) const
{
	ListItem<TileSet*>* item = data.tilesets.start;
	TileSet* set = item->data;

	while (item)
	{
		if (id < item->data->firstgid)
		{
			set = item->prev->data;
			break;
		}
		set = item->data;
		item = item->next;
	}

	return set;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int id) const
{
	SDL_Rect rect = { 0 };

	// L04: DONE 7: Get relative Tile rectangle
	int relativeId = id - firstgid;
	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + ((rect.w + spacing) * (relativeId % numTilesWidth));
	rect.y = margin + ((rect.h + spacing) * (relativeId / numTilesWidth));
	
	return rect;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L03: DONE 2: Make sure you clean up any memory allocated from tilesets/map
    // Remove all tilesets
	ListItem<TileSet*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		app->tex->UnLoad(item->data->texture);
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.Clear();

	// L04: DONE 2: clean up all layer data
	// Remove all layers
	ListItem<MapLayer*>* item2;
	item2 = data.layers.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	data.layers.Clear();

	data.orgLayers.Clear();

	// Clean up the pugui tree
	mapFile.reset();

    return true;
}

// Load new map
bool Map::Load(const char* filename)
{
    bool ret = true;
    SString tmp("%s/%s", folder.GetString(), filename);

    pugi::xml_parse_result result = mapFile.load_file(tmp.GetString());

    if(result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", filename, result.description());
        ret = false;
    }

	// Load general info
    if(ret == true)
    {
        // L03: DONE 3: Create and call a private function to load and fill all your map data
		ret = LoadMap();
	}

    // L03: DONE 4: Create and call a private function to load a tileset
    // remember to support more any number of tilesets!
	pugi::xml_node tileset;
	for (tileset = mapFile.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true) ret = LoadTilesetDetails(tileset, set);

		if (ret == true) ret = LoadTilesetImage(tileset, set);

		data.tilesets.Add(set);
	}

	// L04: DONE 4: Iterate all layers and load each of them
	// Load layer info
	pugi::xml_node layer;
	for (layer = mapFile.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if (ret == true) data.layers.Add(lay);

		if (strcmp(lay->name.GetString(), "colliders") == 0)
		{
			LoadColliders(lay);
		}
	}
    
    if(ret == true)
    {
        // L03: DONE 5: LOG all the data loaded iterate all tilesets and LOG everything
		//LOG("Successfully parsed map XML file: %s", filename);
		//LOG("width: %d height: %d", data.width, data.height);
		//LOG("tile_width: %d tile_height: %d", data.tileWidth, data.tileHeight);

		ListItem<TileSet*>* item = data.tilesets.start;
		while (item != NULL)
		{
			TileSet* s = item->data;
			//LOG("Tileset ----");
			//LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			//LOG("tile width: %d tile height: %d", s->tileWidth, s->tileHeight);
			//LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item = item->next;
		}

		// L04: DONE 4: LOG the info for each loaded layer
		ListItem<MapLayer*>* itemLayer = data.layers.start;
		while (itemLayer != NULL)
		{
			MapLayer* l = itemLayer->data;
			LOG("Layer ----");
			LOG("name: %s", l->name.GetString());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			itemLayer = itemLayer->next;
		}
	}

    mapLoaded = ret;

	tmp.Clear();

    return ret;
}

// L03: DONE: Load map general properties
bool Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE: Load map general properties
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tileWidth = map.attribute("tilewidth").as_int();
		data.tileHeight = map.attribute("tileheight").as_int();
		SString bg_color(map.attribute("backgroundcolor").as_string());

		data.backgroundColor.r = 0;
		data.backgroundColor.g = 0;
		data.backgroundColor.b = 0;
		data.backgroundColor.a = 0;
		/*
		if (bg_color.Length() > 0)
		{
			SString red, green, blue;
			bg_color.SubString(1, 2, red);
			bg_color.SubString(3, 4, green);
			bg_color.SubString(5, 6, blue);

			int v = 0;

			sscanf_s(red.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.r = v;

			sscanf_s(green.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.g = v;

			sscanf_s(blue.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.b = v;
		}
		*/

		SString orientation(map.attribute("orientation").as_string());

		if (orientation == "orthogonal") data.type = MAPTYPE_ORTHOGONAL;
		else if (orientation == "isometric") data.type = MAPTYPE_ISOMETRIC;
		else if (orientation == "staggered") data.type = MAPTYPE_STAGGERED;
		else data.type = MAPTYPE_UNKNOWN;
	}

	return ret;
}

// L03: DONE: Load Tileset attributes
bool Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	
	// L03: DONE: Load Tileset attributes
	set->name.Create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tileWidth = tileset_node.attribute("tilewidth").as_int();
	set->tileHeight = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if (offset != NULL)
	{
		//set->offsetX = offset.attribute("x").as_int();
		//set->offsetY = offset.attribute("y").as_int();
	}
	else
	{
		set->offsetX = 0;
		set->offsetY = 0;
	}

	return ret;
}

// L03: DONE: Load Tileset image
bool Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE: Load Tileset image
		set->texture = app->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->texWidth = image.attribute("width").as_int();

		if (set->texWidth <= 0)
		{
			set->texWidth = w;
		}

		set->texHeight = image.attribute("height").as_int();

		if (set->texHeight <= 0)
		{
			set->texHeight = h;
		}

		set->numTilesWidth = set->texWidth / set->tileWidth;
		set->numTilesHeight = set->texHeight / set->tileHeight;
	}

	return ret;
}

// L04: DONE 3: Create the definition for a function that loads a single layer
bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;
	
	// L04: DONE 3: Load a single layer
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	pugi::xml_node layerData = node.child("data");

	if (layerData == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layer);
	}
	else
	{
		layer->data = new uint[layer->width * layer->height];
		memset(layer->data, 0, layer->width * layer->height);
		
		int i = 0;
		for (pugi::xml_node tile = layerData.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data[i++] = tile.attribute("gid").as_int(0);
		}
	}

	// Load layer properties
	LoadProperties(node, layer->properties);

	return ret;
}	

// L06: DONE 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if (data != NULL)
	{
		pugi::xml_node prop;

		for (prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list.Add(p);
		}
	}
	
	return ret;
}

// L12b: Create walkability map for pathfinding
bool Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	bool ret = false;
	ListItem<MapLayer*>* item;
	item = data.layers.start;

	for(item = data.layers.start; item != NULL; item = item->next)
	{
		MapLayer* layer = item->data;

		if(layer->properties.GetProperty("Navigation", 0) == 0)
			continue;

		uchar* map = new uchar[layer->width*layer->height];
		memset(map, 1, layer->width*layer->height);

		for(int y = 0; y < data.height; ++y)
		{
			for(int x = 0; x < data.width; ++x)
			{
				int i = (y*layer->width) + x;

				int tileId = layer->Get(x, y);
				TileSet* tileset = (tileId > 0) ? GetTilesetFromTileId(tileId) : NULL;
				
				if(tileset != NULL)
				{
					map[i] = (tileId - tileset->firstgid) > 0 ? 0 : 1;
				}
			}
		}

		*buffer = map;
		width = data.width;
		height = data.height;
		ret = true;

		break;
	}

	return ret;
}

void Map::LoadColliders(MapLayer* layer)
{
	for (int y = 0; y < data.height; ++y)
	{
		for (int x = 0; x < data.width; ++x)
		{
			uint tileId = layer->Get(x, y);

			if (tileId > 0)
			{
				TileSet* set = data.tilesets.end->data;

				SDL_Rect r = set->GetTileRect(tileId);

				iPoint pos = MapToWorld(x, y);

				r.x = pos.x * SCALE;
				r.y = pos.y * SCALE;
				r.w *= SCALE;
				r.h *= SCALE;

				if (app->scene->world->GetPlace() == Places::MAIN_VILLAGE)
				{
					switch (tileId)
					{
					case 5640: // HOUSES
						app->scene->world->houses.Add(r);
						break;

					case 5639: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 5638: // LOCATION 1
						app->scene->world->location1.Add(r);
						break;

					case 5637: // LOCATION 2
						app->scene->world->location2.Add(r);
						break;

					case 5636: // LOCATION 3
						app->scene->world->location3.Add(r);
						break;
					case 5635:
						app->scene->world->tavern.Add(r);
						break;
					case 5634:
						app->scene->world->shop.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::HOUSE)
				{
					switch (tileId)
					{
					case 527: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 526: // VILLAGE
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::ENEMY_FIELD)
				{
					switch (tileId)
					{
					case 1799: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;
					case 1796: // MAIN VILLAGE
						app->scene->world->location3.Add(r);
						break;
					case 1797: // GRASSY LANDS
						app->scene->world->location2.Add(r);
						break;
					case 1798: // AUTUM FALLS
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::TAVERN)
				{
					switch (tileId)
					{
					case 591: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 590: // VILLAGE
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::SHOP)
				{
					switch (tileId)
					{
					case 1551: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 1550: // VILLAGE
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::GRASSY_LAND_1)
				{
					switch (tileId)
					{
					case 2890: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 2888: // GRASSY LAND 2
						app->scene->world->location1.Add(r);
						break;

					case 2889: // ENEMY FIELD
						app->scene->world->location2.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::GRASSY_LAND_2)
				{
					switch (tileId)
					{
					case 1799: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;
					case 1796: // GRASSY LANDS 1
						app->scene->world->location1.Add(r);
						break;
					case 1798: // GRASSY LANDS 3
						app->scene->world->location2.Add(r);
						break;
					case 1795: // IMMERSIVE GROUND
						app->scene->world->collisionsOnOff.Add(r);
						break;
					case 1800: //Whatever we need
						app->scene->world->lilipadPuzzle1->riverTracks.Add({ r, true });
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::GRASSY_LAND_3)
				{
					switch (tileId)
					{
					case 2890: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;
					case 2889: // GRASSY LANDS 2
						app->scene->world->location2.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::AUTUM_FALL_1)
				{
					switch (tileId)
					{
					case 2890: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 2889: // ENEMY FIELD
						app->scene->world->location1.Add(r);
						break;

					case 2888: // AUTUMN FALLS 2
						app->scene->world->location2.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::AUTUM_FALL_2)
				{
					switch (tileId)
					{
					case 2122: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 2121: // AUTUMN FALL 1
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::MOSSY_ROCKS_1)
				{
					switch (tileId)
					{
					case 426: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 425: // MAIN_VILLAGE
						app->scene->world->location1.Add(r);
						break;

					case 424: // MOSSY ROCKS 2
						app->scene->world->location2.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::MOSSY_ROCKS_2)
				{
					switch (tileId)
					{
					case 426: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 425: // MOSSY ROCKS 1
						app->scene->world->location1.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::GOLEM_STONES)
				{
					switch (tileId)
					{
					case 2890: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 2887: // BOSS_SANCTUARY
						app->scene->world->location1.Add(r);
						break;

					case 2888: // MAIN_VILLAGE
						app->scene->world->location2.Add(r);
						break;
					}
				}
				else if (app->scene->world->GetPlace() == Places::CAVE)
				{
					switch (tileId)
					{
					case 1877: // COLLIDER
						app->scene->world->collisions.Add(r);
						break;

					case 1876: // GOLEM STONES 1
						app->scene->world->location1.Add(r);
						break;
					}
				}
			}
		}
	}
}